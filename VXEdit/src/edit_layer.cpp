#include "edit_layer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/application.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include "ivx/scene/scene.h"
#include "ivx/scene/scriptable_entity.h"
#include "panel/component_registry.h"
#include "panel/layout.h"
#include "scripts/controller.h"
#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <imgui.h>

namespace ivx {

static constexpr uint32_t WIDTH = 1280;
static constexpr uint32_t HEIGHT = 720;

EditLayer::EditLayer() : Layer("EditLayer") {
}

void EditLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    // The editor layout is stored next to the assets instead of the process
    // working directory, so it survives being launched from anywhere.
    Application::get().imgui_layer()->set_ini_filename(LAYOUT);
    // A saved ini means the user already has a layout: do not rebuild the
    // default one over it (Reset Layout in the Layout menu still does).
    layout_initialized_ = std::filesystem::exists(LAYOUT);

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig cfg;
    // cfg.PixelSnapH = true;
    ImFont *font =
        io.Fonts->AddFontFromFileTTF(FONTS "/consola.ttf", 16.0f, &cfg);
    IVX_CORE_ASSERT(font, "Failed to load Consolas");
    io.FontDefault = font;

    // static uint32_t WIDTH = Application::get().get_window().get_width();
    // static uint32_t HEIGHT = Application::get().get_window().get_height();

    texture_ = Texture2D::create(TEXTURES "tilemap.png");
    sub_texture_ =
        SubTexture2D::create(texture_, {2.f, 1.f}, {16.f, 16.f}, {2.f, 1.f});

    FrameBufferSpecification spec;
    spec.width = WIDTH;
    spec.height = HEIGHT;

    frame_buffer_ = FrameBuffer::create(spec);

    scene_ = make_ref<Scene>();
    context_.scene = scene_;
    square_entity_ = scene_->create_entity("Colorful Square");

    // auto transform = [](const glm::vec3 &translation = glm::vec3{0.f},
    //                     float rotation = 0.f,
    //                     const glm::vec2 &scale = glm::vec2{1.f}) -> glm::mat4
    //                     {
    //     return glm::translate(glm::mat4{1.f}, translation) *
    //            glm::rotate(glm::mat4{1.f},
    //                        glm::radians(rotation),
    //                        glm::vec3{0.f, 0.f, 1.f}) *
    //            glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
    // };

    // Scale stays 1: the quad size lives in SpriteRendererComponent so that
    // children are not scaled by their parent.
    square_entity_.get_component<TransformComponent>().translation =
        glm::vec3{0.f};
    square_entity_.get_component<TransformComponent>().rotation = 0.f;
    square_entity_.get_component<TransformComponent>().scale =
        glm::vec2{1.f, 1.f};

    square_entity_.add_component<SpriteRendererComponent>(
        sub_texture_,
        1.f,
        glm::vec4{1.f, 1.f, 1.f, 1.f},
        glm::vec2{400.f, 200.f});

    camera_entity_ = scene_->create_entity("Camera Entity");
    {
        auto &camera = camera_entity_.add_component<CameraComponent>();
        camera.camera.set_orthographic_size(static_cast<float>(HEIGHT));
        camera.camera.set_projection_type(
            SceneCamera::ProjectionType::Orthographic);
        camera.primary = true;
        // Initial pose: one unit in front of the sprite plane (z = 0), looking
        // down -Z. near/far are distances, so 0.01 .. 1000 covers it.
        camera_entity_.get_component<TransformComponent>().translation =
            glm::vec3{0.f, 0.f, 1.f};
    }
    camera_entity_.add_component<NativeScriptComponent>().bind<Controller>();

    second_camera_entity_ = scene_->create_entity("CameraB Entity");
    {
        auto &camera = second_camera_entity_.add_component<CameraComponent>();
        // Half the height, fixed aspect: acts as a "close-up" camera.
        camera.camera.set_orthographic_size(static_cast<float>(HEIGHT) / 2.f);
        camera.camera.set_projection_type(
            SceneCamera::ProjectionType::Orthographic);
        camera.primary = false;
        camera.fixed_aspect_ratio = true;
        second_camera_entity_.get_component<TransformComponent>().translation =
            glm::vec3{0.f, 0.f, 1.f};
    }

    setup_panels();
}

void EditLayer::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void EditLayer::on_update(float dt) {
    IVX_PROFILE_FUNCTION();

    // Keeps collecting log output even when the Console panel is closed.
    if (console_panel_) {
        console_panel_->pump();
    }

    if (visible_) {
        const auto &spec = frame_buffer_->get_specification();
        if (spec.width != viewport_size_.x || spec.height != viewport_size_.y) {
            uint32_t w = static_cast<uint32_t>(viewport_size_.x);
            uint32_t h = static_cast<uint32_t>(viewport_size_.y);

            scene_->on_viewport_resize(w, h);
            frame_buffer_->resize(w, h);
        }
    }

    // update
    if (auto *script = camera_entity_.try_get_component<NativeScriptComponent>()) {
        script->enabled = viewport_focused_;
    }

    scene_->on_update(dt);

    // render
    Renderer2D::reset_stats();

    frame_buffer_->bind();

    {
        // clear
        IVX_PROFILE_SCOPE("Renderer prep");
        RenderCommand::set_clear_color(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});
        RenderCommand::clear();
    }

    {
        // draw
        IVX_PROFILE_SCOPE("Renderer draw");

        scene_->on_render();
    }

    frame_buffer_->unbind();
}

void EditLayer::on_event(Event &event) {
    if (event.is_in_category(EventCategory::kEventCategoryMouse |
                             EventCategory::kEventCategoryKeyboard)) {
        // IVX_VERBOSE("{}", event.to_string());
    }
}

void EditLayer::setup_panels() {
    IVX_PROFILE_FUNCTION();

    register_all_components();

    // Deterministic draw order: Hierarchy -> Viewport -> Inspector -> Camera.
    scene_hierarchy_panel_ = panels_.add<SceneHierarchyPanel>(context_);

    auto viewport = panels_.add<LambdaPanel>(
        "Viewport", [this]() { draw_viewport_content(); });
    viewport->set_pre_begin([]() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
    }).set_post_end([]() { ImGui::PopStyleVar(); }).set_on_hidden([this]() {
        visible_ = false;
        viewport_focused_ = false;
        viewport_hovered_ = false;
        Application::get().imgui_layer()->block_event(true);
    });

    inspector_panel_ = panels_.add<InspectorPanel>(context_);
    camera_panel_ = panels_.add<CameraPanel>(context_);
    console_panel_ = panels_.add<ConsolePanel>();
}

void EditLayer::on_render() {
    IVX_PROFILE_FUNCTION();

    // replace fullscreen host window + DockSpace
    const ImGuiID dockspace_id =
        ImGui::DockSpaceOverViewport(0,
                                     ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

    // First frame (or after "Reset Layout") build the default arrangement.
    if (!layout_initialized_ || reset_layout_requested_) {
        setup_default_layout(dockspace_id);
        layout_initialized_ = true;
        reset_layout_requested_ = false;
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                Application::get().close();
            }
            ImGui::EndMenu();
        }
        panels_.draw_view_menu();
        if (ImGui::BeginMenu("Layout")) {
            if (ImGui::MenuItem("Reset Layout")) {
                reset_layout_requested_ = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    panels_.on_render();
}

void EditLayer::draw_viewport_content() {
    viewport_focused_ = ImGui::IsWindowFocused();
    viewport_hovered_ = ImGui::IsWindowHovered();
    Application::get().imgui_layer()->block_event(!viewport_focused_ ||
                                                  !viewport_hovered_);

    visible_ = true;

    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    float w = std::max(viewport_size.x, 1.f);
    float h = std::max(viewport_size.y, 1.f);

    viewport_size_ = {w, h};

    ImGui::Image(
        static_cast<ImTextureID>(frame_buffer_->get_color_attachment()),
        viewport_size_,
        ImVec2{0.0f, 1.0f},
        ImVec2{1.0f, 0.0f});
}
} // namespace ivx
