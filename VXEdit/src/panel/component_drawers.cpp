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

// ---- camera ----------------------------------------------------------------
// A camera is just a component, so everything about it lives here instead of in
// a dedicated panel: projection parameters, primary-camera exclusivity, the
// controller script shortcut and the view helpers.

bool scene_has_primary_camera(Entity entity) {
    Scene *scene = entity.scene();
    if (scene == nullptr) {
        return false;
    }
    for (auto [handle, camera] :
         scene->registry().view<CameraComponent>().each()) {
        if (camera.primary) {
            return true;
        }
    }
    return false;
}

// "Add Component -> Camera" on any entity: a camera on a player, a turret or a
// cutscene rig is just another component.
//
// Adding a component must not move the entity and must not touch its transform:
// the camera ends up exactly where the entity already was (usually the origin).
void add_camera(Entity entity) {
    auto &camera = entity.add_component<CameraComponent>();
    camera.camera.set_orthographic_size(720.f);
    camera.camera.set_projection_type(
        SceneCamera::ProjectionType::Orthographic);

    // Become the visible camera only when the scene has none: adding a camera
    // must not silently change what the viewport shows.
    camera.primary = !scene_has_primary_camera(entity);
}

// Exactly one camera is primary: the renderer takes the first one it finds, so
// two checked boxes would be ambiguous.
void make_primary(EditorContext &context, Entity entity) {
    if (!context.scene) {
        return;
    }
    const entt::entity wanted = entity ? entity.handle() : entt::null;
    for (auto [handle, camera] :
         context.scene->registry().view<CameraComponent>().each()) {
        camera.primary = (handle == wanted);
    }
}

// Points the camera at the selected entity (and frames it when orthographic).
void focus_selection(Entity camera_entity,
                     EditorContext &context,
                     bool also_frame) {
    const Entity selection = context.selected_entity();
    if (!selection) {
        return;
    }

    auto *camera_transform =
        camera_entity.try_get_component<TransformComponent>();
    const auto *target_transform =
        selection.try_get_component<TransformComponent>();
    if (camera_transform == nullptr || target_transform == nullptr) {
        return;
    }

    // Cameras live in world space, keep the camera's own depth.
    camera_transform->translation.x = target_transform->world[3][0];
    camera_transform->translation.y = target_transform->world[3][1];

    if (!also_frame) {
        return;
    }

    // Orthographic only: choose a size that leaves a margin around the sprite.
    auto *camera = camera_entity.try_get_component<CameraComponent>();
    if (camera == nullptr || camera->camera.get_projection_type() !=
                                 SceneCamera::ProjectionType::Orthographic) {
        return;
    }

    const auto *sprite = selection.try_get_component<SpriteRendererComponent>();
    const float extent =
        sprite != nullptr ? glm::max(sprite->size.x, sprite->size.y) : 200.f;
    camera->camera.set_orthographic_size(extent * 1.5f);
}

// Keeps the "why is WASD dead?" question from happening: the camera only reacts
// to input while a controller script is attached. When it is attached the row
// stays a single dim line, because the Native Script section right below owns
// the controls.
void draw_camera_controller(Entity entity) {
    auto *script = entity.try_get_component<NativeScriptComponent>();
    if (script != nullptr && script->is_bound()) {
        ImGui::TextDisabled("WASD / wheel: %s", script->script_name.c_str());
        return;
    }

    ImGui::TextDisabled("WASD / wheel: none");
    ImGui::SameLine();

    // Only offer the binding when the module actually exports it.
    const ScriptEntry *controller = nullptr;
    for (const ScriptEntry &entry : ScriptRegistry::get().entries()) {
        if (entry.name == "CameraController") {
            controller = &entry;
            break;
        }
    }
    if (controller == nullptr) {
        ImGui::TextDisabled("(VXScripts.dll not loaded yet)");
        return;
    }

    if (ImGui::SmallButton("Attach CameraController")) {
        if (script == nullptr) {
            entity.add_component<NativeScriptComponent>();
        }
        ScriptRegistry::get().bind_by_name(controller->name, entity);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Adds a Native Script component bound to CameraController:\n"
            "WASD moves, Q/E rotates, the wheel zooms.");
    }
}

void draw_camera(Entity entity,
                 CameraComponent &camera,
                 EditorContext &context) {
    SceneCamera &scene_camera = camera.camera;

    if (camera.primary) {
        ImGui::TextColored(ImVec4{0.4f, 0.8f, 0.4f, 1.f}, "[primary]");
        ImGui::SameLine();
        ImGui::TextDisabled("this camera renders the viewport");
    }

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
        // FOV is already in degrees (see SceneCamera): no conversion here,
        // otherwise the slider would show 45 as 2578 deg.
        float fov = scene_camera.get_perspective_vertical_fov();
        if (ImGui::SliderFloat("FOV", &fov, 1.f, 179.f, "%.1f deg")) {
            scene_camera.set_perspective_vertical_fov(fov);
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

    // The checkbox routes through make_primary() so turning one camera on turns
    // the others off, matching what the renderer expects.
    bool primary = camera.primary;
    if (ImGui::Checkbox("Primary", &primary)) {
        if (primary) {
            make_primary(context, entity);
        } else {
            camera.primary = false;
        }
    }

    ImGui::Separator();

    // ---- view helpers ----
    if (const auto *transform =
            entity.try_get_component<TransformComponent>()) {
        ImGui::TextDisabled("World position: (%.1f, %.1f, %.1f)",
                            transform->world[3][0],
                            transform->world[3][1],
                            transform->world[3][2]);
    }

    const bool has_selection = context.has_selection();
    ImGui::BeginDisabled(!has_selection);
    if (ImGui::Button("Focus Selection")) {
        focus_selection(entity, context, /*also_frame*/ false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Frame Selection")) {
        focus_selection(entity, context, /*also_frame*/ true);
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Moves the camera onto the selected entity and sizes it to fit.");
    }

    ImGui::Separator();

    // ---- derived numbers, for when "nothing renders" ----
    if (scene_camera.get_projection_type() ==
        SceneCamera::ProjectionType::Orthographic) {
        ImGui::Text("Visible area: %.1f x %.1f (world units)",
                    scene_camera.get_orthographic_width(),
                    scene_camera.get_orthographic_height());
    } else {
        ImGui::Text("Vertical FOV: %.1f deg",
                    scene_camera.get_perspective_vertical_fov());
    }

    if (ImGui::TreeNode("Projection matrix (read-only)")) {
        const glm::mat4 projection = scene_camera.get_proj_matrix();
        ImGui::Text("m00 %.6f   m11 %.6f", projection[0][0], projection[1][1]);
        ImGui::Text("m22 %.6f   m32 %.6f", projection[2][2], projection[3][2]);
        ImGui::TreePop();
    }

    ImGui::Separator();

    draw_camera_controller(entity);
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

// Removing the component has to release the instance first: it lives in the
// script module and nothing else would call on_destroy() on it.
// remove_component() alone would just drop the pointers (a leak), so the entry
// gets a custom remove.
void remove_script(Entity entity) {
    if (auto *script = entity.try_get_component<NativeScriptComponent>()) {
        script->unbind();
    }
    entity.remove_component<NativeScriptComponent>();
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
    // A camera is just a component: it can be added, edited and removed like
    // any other. Scene Hierarchy's "+ Entity" menu is the shortcut for a fresh
    // camera entity.
    registry.add<CameraComponent>("Camera", draw_camera, {}, add_camera);
    // Removable like any other component; the custom remove unbinds first.
    registry.add<NativeScriptComponent>(
        "Native Script", draw_script, {}, {}, remove_script);

    IVX_CORE_DEBUG("Registered {} component type(s)",
                   registry.entries().size());
}

} // namespace ivx
