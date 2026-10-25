#include "component_registry.h"

#include "ivx/core/logger.h"
#include "ivx/scene/components.h"
#include "ivx/scene/scriptable_entity.h"
#include "scripts/script_registry.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <string>

namespace ivx {
namespace {

// Small helper: a DragFloat row with a reset button on the right.
void drag_vec2(const char *label, glm::vec2 &value, float speed) {
    ImGui::DragFloat2(label, glm::value_ptr(value), speed);
}

void draw_transform(Entity /*entity*/,
                    TransformComponent &transform,
                    EditorContext & /*context*/) {
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

void draw_sprite(Entity /*entity*/,
                 SpriteRendererComponent &sprite,
                 EditorContext & /*context*/) {
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

void draw_camera(Entity /*entity*/,
                 CameraComponent &camera,
                 EditorContext & /*context*/) {
    SceneCamera &scene_camera = camera.camera;

    // Hazel-style: one projection switch, each type keeps its own parameters.
    const char *types[] = {"Perspective", "Orthographic"};
    int type = static_cast<int>(scene_camera.get_projection_type());
    if (ImGui::Combo("Projection", &type, types, IM_ARRAYSIZE(types))) {
        scene_camera.set_projection_type(
            static_cast<SceneCamera::ProjectionType>(type));
    }

    if (scene_camera.get_projection_type() ==
        SceneCamera::ProjectionType::Orthographic) {
        float size = scene_camera.get_orthographic_size();
        if (ImGui::DragFloat("Size", &size, 1.f, 0.01f, 100000.f)) {
            scene_camera.set_orthographic_size(size);
        }
        float near_clip = scene_camera.get_orthographic_near();
        if (ImGui::DragFloat("Near", &near_clip, 0.01f, 0.001f, 10000.f)) {
            scene_camera.set_orthographic_near(near_clip);
        }
        float far_clip = scene_camera.get_orthographic_far();
        if (ImGui::DragFloat("Far", &far_clip, 1.f, 0.01f, 100000.f)) {
            scene_camera.set_orthographic_far(far_clip);
        }
    } else {
        float fov = glm::degrees(scene_camera.get_perspective_vertical_fov());
        if (ImGui::SliderFloat("FOV", &fov, 1.f, 179.f, "%.1f deg")) {
            scene_camera.set_perspective_vertical_fov(glm::radians(fov));
        }
        float near_clip = scene_camera.get_perspective_near();
        if (ImGui::DragFloat("Near", &near_clip, 0.01f, 0.001f, 10000.f)) {
            scene_camera.set_perspective_near(near_clip);
        }
        float far_clip = scene_camera.get_perspective_far();
        if (ImGui::DragFloat("Far", &far_clip, 1.f, 0.01f, 100000.f)) {
            scene_camera.set_perspective_far(far_clip);
        }
    }

    ImGui::TextDisabled("Aspect %.3f (from viewport)",
                        scene_camera.get_aspect_ratio());
    ImGui::Checkbox("Fixed Aspect Ratio", &camera.fixed_aspect_ratio);

    ImGui::Separator();

    ImGui::Checkbox("Primary", &camera.primary);
}

void draw_script(Entity entity,
                 NativeScriptComponent &script,
                 EditorContext & /*context*/) {
    const ScriptRegistry &scripts = ScriptRegistry::get();

    const char *bound_name = scripts.name_of(script.instantiate);

    if (script.is_bound()) {
        ImGui::Text("Script: %s",
                    bound_name != nullptr ? bound_name : "<unknown>");
    } else {
        ImGui::TextDisabled("No script bound");
    }

    // Picking a script binds it (and destroys the previous instance).
    if (ImGui::BeginCombo(
            "##script",
            script.is_bound()
                ? (bound_name != nullptr ? bound_name : "<unknown>")
                : "Select script...")) {
        for (const ScriptEntry &entry : scripts.entries()) {
            const bool selected =
                bound_name != nullptr && entry.name == bound_name;
            if (ImGui::Selectable(entry.name.c_str(), selected)) {
                scripts.bind_by_name(entry.name, entity);
            }
        }
        ImGui::EndCombo();
    }

    // An unbound script cannot run, so its toggle would only be misleading.
    ImGui::BeginDisabled(!script.is_bound());
    ImGui::Checkbox("Enabled", &script.enabled);
    ImGui::EndDisabled();

    if (script.is_bound()) {
        ImGui::SameLine();
        if (ImGui::Button("Unbind")) {
            const auto *tag = entity.try_get_component<TagComponent>();
            IVX_INFO("Unbound script from '{}'",
                     tag != nullptr ? tag->tag : "<unnamed>");
            script.unbind();
        }
    }
}

} // namespace

void register_all_components() {
    auto &registry = ComponentRegistry::get();
    if (!registry.entries().empty()) {
        return; // already registered
    }

    // Tag is edited by the inspector header (the entity name), so it has no
    // section of its own and cannot be added/removed.
    registry.add<TagComponent>(
        "Tag",
        /*draw*/ {},
        {.removable = false, .addable = false, .draw_in_inspector = false});
    registry.add<TransformComponent>(
        "Transform", draw_transform, {.removable = false});
    registry.add<SpriteRendererComponent>("Sprite Renderer", draw_sprite);
    // Cameras are entities, not something you bolt onto a sprite: they are
    // created from the Camera panel. Still editable/removable when present.
    registry.add<CameraComponent>("Camera", draw_camera, {.addable = false});
    registry.add<NativeScriptComponent>(
        "Native Script", draw_script, {.removable = false});

    IVX_CORE_DEBUG("Registered {} component type(s)",
                   registry.entries().size());
}

} // namespace ivx
