#include "component_registry.h"

#include "ivx/core/logger.h"
#include "ivx/scene/components.h"
#include "ivx/scene/scriptable_entity.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <string>

namespace ivx {
namespace {

// Small helper: a DragFloat row with a reset button on the right.
void drag_vec2(const char *label, glm::vec2 &value, float speed) {
    ImGui::DragFloat2(label, glm::value_ptr(value), speed);
}

void draw_tag(TagComponent &tag, EditorContext & /*context*/) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", tag.tag.c_str());
    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
        tag.tag = buffer;
    }
}

void draw_transform(TransformComponent &transform, EditorContext & /*context*/) {
    ImGui::DragFloat3("Position", glm::value_ptr(transform.translation), 0.5f);
    ImGui::DragFloat("Rotation", &transform.rotation, 0.5f);
    drag_vec2("Scale", transform.scale, 0.01f);

    if (ImGui::Button("Reset Transform")) {
        transform.translation = glm::vec3{0.f};
        transform.rotation = 0.f;
        transform.scale = glm::vec2{1.f};
    }

    const glm::mat4 &world = transform.world;
    if (ImGui::TreeNode("World matrix (read-only)")) {
        ImGui::Text("position: (%.2f, %.2f, %.2f)",
                    world[3][0],
                    world[3][1],
                    world[3][2]);
        ImGui::TreePop();
    }
}

void draw_sprite(SpriteRendererComponent &sprite, EditorContext & /*context*/) {
    ImGui::DragFloat2("Size", glm::value_ptr(sprite.size), 0.5f);
    ImGui::DragFloat("Tiling", &sprite.tiling_factor, 0.05f, 0.01f, 64.f);
    ImGui::ColorEdit4("Color", glm::value_ptr(sprite.color));

    // No asset browser yet, so the texture is reported read-only.
    if (sprite.sub_texture) {
        ImGui::TextDisabled("SubTexture (id %u)",
                            sprite.sub_texture->get_texture()
                                ? sprite.sub_texture->get_texture()->get_id()
                                : 0u);
    } else if (sprite.texture) {
        ImGui::TextDisabled("Texture (id %u, %ux%u)",
                            sprite.texture->get_id(),
                            sprite.texture->get_width(),
                            sprite.texture->get_height());
    } else {
        ImGui::TextDisabled("No texture (flat color)");
    }
}

void draw_camera(CameraComponent &camera, EditorContext & /*context*/) {
    SceneCamera &scene_camera = camera.camera;

    int mode = scene_camera.get_mode() == SceneCamera::Mode::Orthogonal ? 0 : 1;
    const char *modes[] = {"Orthogonal", "Perspective"};
    if (ImGui::Combo("Mode", &mode, modes, IM_ARRAYSIZE(modes))) {
        scene_camera.set_mode(mode == 0 ? SceneCamera::Mode::Orthogonal
                                        : SceneCamera::Mode::Perspective);
    }

    if (scene_camera.get_mode() == SceneCamera::Mode::Orthogonal) {
        float size = scene_camera.get_size();
        if (ImGui::DragFloat("Size", &size, 1.f, 1.f, 20000.f)) {
            scene_camera.set_size(size);
        }
        float zoom = scene_camera.get_zoom_level();
        if (ImGui::DragFloat("Zoom", &zoom, 0.05f, 0.05f, 20.f)) {
            scene_camera.set_zoom_level(zoom);
        }
    } else {
        float fov = glm::degrees(scene_camera.get_fov());
        if (ImGui::DragFloat("FOV", &fov, 0.5f, 1.f, 179.f)) {
            scene_camera.set_fov(glm::radians(fov));
        }
    }

    float near_plane = scene_camera.get_near();
    if (ImGui::DragFloat("Near", &near_plane, 0.01f, -10000.f, 10000.f)) {
        scene_camera.set_near(near_plane);
    }
    float far_plane = scene_camera.get_far();
    if (ImGui::DragFloat("Far", &far_plane, 0.01f, -10000.f, 10000.f)) {
        scene_camera.set_far(far_plane);
    }

    ImGui::Checkbox("Primary", &camera.primary);
    ImGui::Checkbox("Fixed Aspect Ratio", &camera.fixed_aspect_ratio);
}

void draw_script(NativeScriptComponent &script, EditorContext & /*context*/) {
    if (script.instantiate != nullptr) {
        ImGui::TextDisabled("Script bound");
    } else {
        ImGui::TextDisabled("No script bound");
    }
    ImGui::Checkbox("Enabled", &script.enabled);
}

} // namespace

void register_all_components() {
    auto &registry = ComponentRegistry::get();
    if (!registry.entries().empty()) {
        return; // already registered
    }

    registry.add<TagComponent>("Tag", draw_tag, /*removable*/ false);
    registry.add<TransformComponent>(
        "Transform", draw_transform, /*removable*/ false);
    registry.add<SpriteRendererComponent>("Sprite Renderer", draw_sprite);
    registry.add<CameraComponent>(
        "Camera",
        draw_camera,
        /*removable*/ true,
        // CameraComponent has no default constructor; six arguments select the
        // orthogonal overload unambiguously.
        [](Entity entity) {
            entity.add_component<CameraComponent>(720.f, // size
                                                  -1.f,  // near
                                                  1.f,   // far
                                                  1.f,   // zoom
                                                  false, // primary
                                                  false); // fixed aspect
        });
    registry.add<NativeScriptComponent>(
        "Native Script", draw_script, /*removable*/ false);

    IVX_CORE_DEBUG("Registered {} component type(s)", registry.entries().size());
}

} // namespace ivx
