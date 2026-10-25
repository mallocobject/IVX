#include "camera_panel.h"

#include "component_registry.h"
#include "ivx/scene/components.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <string>

namespace ivx {
namespace {
std::string entity_label(Entity entity) {
    const auto *tag = entity.try_get_component<TagComponent>();
    return tag != nullptr ? tag->tag : std::string{"Camera"};
}
} // namespace

CameraPanel::CameraPanel(EditorContext &context)
    : Panel("Camera"), context_(context) {
    set_default_size(ImVec2{380.f, 420.f});
}

Entity CameraPanel::primary_camera() const {
    if (!context_.scene) {
        return {};
    }
    for (auto [handle, camera] :
         context_.scene->registry().view<CameraComponent>().each()) {
        if (camera.primary) {
            return Entity{handle, context_.scene.get()};
        }
    }
    return {};
}

void CameraPanel::set_primary_camera(Entity entity) {
    if (!context_.scene) {
        return;
    }
    const entt::entity wanted = entity ? entity.handle() : entt::null;
    for (auto [handle, camera] :
         context_.scene->registry().view<CameraComponent>().each()) {
        camera.primary = (handle == wanted);
    }
}

void CameraPanel::create_camera() {
    if (!context_.scene) {
        return;
    }

    Scene &scene = *context_.scene;
    Entity camera_entity = scene.create_entity("Camera");
    auto &camera = camera_entity.add_component<CameraComponent>();
    camera.camera.set_orthographic_size(720.f);
    camera.camera.set_projection_type(
        SceneCamera::ProjectionType::Orthographic);

    // Start one unit in front of the sprite plane (z = 0) so the view is valid
    // for both projections; near/far are distances and already cover it.
    camera_entity.get_component<TransformComponent>().translation =
        glm::vec3{0.f, 0.f, 1.f};

    // Only take over rendering when nothing is primary yet.
    camera.primary = !static_cast<bool>(primary_camera());
    selected_camera_ = camera_entity.handle();
}

void CameraPanel::on_imgui() {
    if (!context_.scene) {
        ImGui::TextUnformatted("No scene");
        return;
    }

    draw_camera_list();
    ImGui::Separator();

    // Keep the edited camera valid; fall back to the primary one.
    const bool selection_valid =
        selected_camera_ != entt::null &&
        context_.scene->registry().valid(selected_camera_) &&
        context_.scene->registry().all_of<CameraComponent>(selected_camera_);
    if (!selection_valid) {
        Entity primary = primary_camera();
        selected_camera_ = primary ? primary.handle() : entt::null;
    }

    if (selected_camera_ == entt::null) {
        ImGui::TextDisabled("No camera in the scene.");
        return;
    }

    draw_camera_details(Entity{selected_camera_, context_.scene.get()});
}

void CameraPanel::draw_camera_list() {
    // Renderer picks the first primary camera; make that visible in the UI.
    ImGui::TextDisabled("Scene cameras (radio = primary)");
    ImGui::SameLine(ImGui::GetContentRegionMax().x - 96.f);
    if (ImGui::SmallButton("+ New Camera")) {
        create_camera();
    }

    for (auto [handle, camera] :
         context_.scene->registry().view<CameraComponent>().each()) {
        Entity entity{handle, context_.scene.get()};

        ImGui::PushID(static_cast<int>(
            static_cast<std::uint32_t>(static_cast<entt::id_type>(handle))));

        if (ImGui::RadioButton(entity_label(entity).c_str(), camera.primary)) {
            set_primary_camera(entity);
            selected_camera_ = handle;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Edit")) {
            selected_camera_ = handle;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s",
                            camera.camera.get_projection_type() ==
                                    SceneCamera::ProjectionType::Orthographic
                                ? "Ortho"
                                : "Persp");

        ImGui::PopID();
    }
}

void CameraPanel::draw_camera_details(Entity entity) {
    auto *camera = entity.try_get_component<CameraComponent>();
    if (camera == nullptr) {
        return;
    }

    ImGui::Text("%s", entity_label(entity).c_str());
    ImGui::SameLine();
    if (camera->primary) {
        ImGui::TextColored(ImVec4{0.4f, 0.8f, 0.4f, 1.f}, "[primary]");
    }

    // ---- view helpers ----
    draw_view_helpers(entity);

    ImGui::Separator();

    // ---- projection (single source of truth: the component registry) ----
    const ComponentEntry *entry = ComponentRegistry::get().find("Camera");
    if (entry != nullptr && entry->draw) {
        entry->draw(entity, context_);
    }

    ImGui::Separator();

    // ---- derived numbers ----
    if (camera->camera.get_projection_type() ==
        SceneCamera::ProjectionType::Orthographic) {
        ImGui::Text("Visible area: %.1f x %.1f (world units)",
                    camera->camera.get_orthographic_width(),
                    camera->camera.get_orthographic_height());
    } else {
        ImGui::Text("Vertical FOV: %.1f deg",
                    glm::degrees(camera->camera.get_perspective_vertical_fov()));
    }

    if (ImGui::TreeNode("Projection matrix (read-only)")) {
        const glm::mat4 projection = camera->camera.get_proj_matrix();
        ImGui::Text("m00 %.6f   m11 %.6f", projection[0][0], projection[1][1]);
        ImGui::Text("m22 %.6f   m32 %.6f", projection[2][2], projection[3][2]);
        ImGui::TreePop();
    }
}

void CameraPanel::draw_view_helpers(Entity entity) {
    const auto *transform = entity.try_get_component<TransformComponent>();
    if (transform != nullptr) {
        ImGui::TextDisabled("World position: (%.1f, %.1f, %.1f)",
                            transform->world[3][0],
                            transform->world[3][1],
                            transform->world[3][2]);
    }

    const Entity selection = context_.selected_entity();
    const bool has_selection = static_cast<bool>(selection);

    ImGui::BeginDisabled(!has_selection);
    if (ImGui::Button("Focus Selection")) {
        focus_selection(entity, /*also_frame*/ false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Frame Selection")) {
        focus_selection(entity, /*also_frame*/ true);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Make Primary")) {
        set_primary_camera(entity);
    }
}

void CameraPanel::focus_selection(Entity camera_entity, bool also_frame) {
    const Entity selection = context_.selected_entity();
    if (!selection) {
        return;
    }

    auto *camera_transform = camera_entity.try_get_component<TransformComponent>();
    const auto *target_transform =
        selection.try_get_component<TransformComponent>();
    if (camera_transform == nullptr || target_transform == nullptr) {
        return;
    }

    // Cameras are placed in world space, keep the camera's own depth.
    camera_transform->translation.x = target_transform->world[3][0];
    camera_transform->translation.y = target_transform->world[3][1];

    if (!also_frame) {
        return;
    }

    // Orthographic only: choose a size that leaves a little margin around the
    // selected sprite.
    auto *camera = camera_entity.try_get_component<CameraComponent>();
    if (camera == nullptr ||
        camera->camera.get_projection_type() !=
            SceneCamera::ProjectionType::Orthographic) {
        return;
    }

    const auto *sprite = selection.try_get_component<SpriteRendererComponent>();
    const float extent =
        sprite != nullptr ? glm::max(sprite->size.x, sprite->size.y) : 200.f;
    camera->camera.set_orthographic_size(extent * 1.5f);
}

} // namespace ivx
