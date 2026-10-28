#include "component_registry.h"

#include "ivx/core/logger.h"
#include "ivx/scene/components.h"
#include "ivx/scene/scriptable_entity.h"
#include "scripts/script_registry.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <string>

namespace ivx {
namespace {

// ---- shared row layout -----------------------------------------------------
// Unity-style rows: the label sits in a fixed left column and the widget fills
// the rest of the row. ImGui draws labels on the right of the widget by
// default, so each row draws its own label; that also lines up every field edge
// down the panel.

float label_column() {
    // Wide enough for the longest row label in use, scaled with the UI.
    return std::ceil(ImGui::CalcTextSize("Fixed Aspect").x) +
           14.f * ui::scale();
}

template <typename Draw>
void label_row(const char *label, Draw &&draw) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(label_column());
    ImGui::SetNextItemWidth(-FLT_MIN);
    draw();
}

// ---- transform -------------------------------------------------------------
// Translation / Rotation / Scale are three rows of axis fields. Every field is
// prefixed with an X/Y/Z marker that reads as the field label and doubles as a
// per-axis reset button, which is the quickest way to fix one axis without
// touching the others.

// Photoshop style: no saturated fills. The marker is flat (the panel gray shows
// through), its glyph uses the muted label gray, and it only lights up with the
// theme accent while the pointer is over it. That keeps the axis identity
// readable without turning the inspector into a colour chart.
void axis_marker(const char *glyph, bool *clicked) {
    const ImGuiStyle &style = ImGui::GetStyle();
    const ImVec2 size{18.f * ui::scale(), ImGui::GetFrameHeight()};

    // The glyph has to be coloured before the button draws, so the hover test
    // runs on the rectangle the button is about to occupy.
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 end{pos.x + size.x, pos.y + size.y};
    const bool hovered = ImGui::IsMouseHoveringRect(pos, end, /*clip*/ true);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.f, 0.f, 0.f, 0.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          style.Colors[ImGuiCol_FrameBgHovered]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          style.Colors[ImGuiCol_FrameBgActive]);
    ImGui::PushStyleColor(ImGuiCol_Text,
                          hovered ? ui::accent() : ui::text_muted());
    *clicked = ImGui::Button(glyph, size);
    ImGui::PopStyleColor(4);
}

// The three fields of one row:  X[ 0.000 ] Y[ 0.000 ] Z[ 0.000 ]
void axis_fields(const char *id,
                 glm::vec3 &value,
                 float speed,
                 float reset_value) {
    constexpr const char *k_axis[] = {"X", "Y", "Z"};

    const float scale = ui::scale();
    const ImGuiStyle &style = ImGui::GetStyle();
    const float marker_width = 18.f * scale;
    // Three markers, three fields and five gaps (marker->field, field->marker,
    // ...).
    const float fields_width =
        (ImGui::GetContentRegionAvail().x - 3.f * marker_width -
         5.f * style.ItemSpacing.x) /
        3.f;
    const float field_width = std::max(24.f * scale, fields_width);

    ImGui::PushID(id);
    for (int axis = 0; axis < 3; ++axis) {
        if (axis > 0) {
            ImGui::SameLine();
        }

        // Per-axis scope: without it the three "##value" fields below would
        // collide in one ID scope (ImGui warns and drops two of them).
        ImGui::PushID(axis);

        bool clicked = false;
        axis_marker(k_axis[axis], &clicked);
        if (clicked) {
            value[axis] = reset_value;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reset %s to %.3g",
                              k_axis[axis],
                              static_cast<double>(reset_value));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(field_width);
        // The marker already names the axis, so the drag field needs no visible
        // label.
        ImGui::DragFloat("##value", &value[axis], speed, 0.f, 0.f, "%.3f");

        ImGui::PopID();
    }
    ImGui::PopID();
}

void draw_transform(Entity /*entity*/,
                    TransformComponent &transform,
                    EditorContext & /*context*/) {
    label_row("Translation", [&] {
        axis_fields("translation", transform.translation, 0.5f, 0.f);
    });
    label_row("Rotation",
              [&] { axis_fields("rotation", transform.rotation, 0.5f, 0.f); });
    label_row("Scale",
              [&] { axis_fields("scale", transform.scale, 0.01f, 1.f); });

    const glm::mat4 &world = transform.world;
    if (ImGui::TreeNode("World matrix (read-only)")) {
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                    world[3][0],
                    world[3][1],
                    world[3][2]);
        ImGui::TreePop();
    }
}

void draw_sprite(Entity /*entity*/,
                 SpriteRendererComponent &sprite,
                 EditorContext & /*context*/) {
    label_row("Size", [&] {
        ImGui::DragFloat2("##size", glm::value_ptr(sprite.size), 0.5f);
    });
    label_row("Tiling", [&] {
        ImGui::DragFloat("##tiling", &sprite.tiling_factor, 0.05f, 0.01f, 64.f);
    });
    label_row("Color", [&] {
        ImGui::ColorEdit4("##color", glm::value_ptr(sprite.color));
    });

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
    label_row("Projection", [&] {
        if (ImGui::Combo("##projection", &type, types, IM_ARRAYSIZE(types))) {
            scene_camera.set_projection_type(
                static_cast<SceneCamera::ProjectionType>(type));
        }
    });

    if (scene_camera.get_projection_type() ==
        SceneCamera::ProjectionType::Orthographic) {
        float size = scene_camera.get_orthographic_size();
        label_row("Size", [&] {
            if (ImGui::DragFloat("##size", &size, 1.f, 0.01f, 100000.f)) {
                scene_camera.set_orthographic_size(size);
            }
        });
        float near_clip = scene_camera.get_orthographic_near();
        label_row("Near", [&] {
            if (ImGui::DragFloat(
                    "##near", &near_clip, 0.01f, 0.001f, 10000.f)) {
                scene_camera.set_orthographic_near(near_clip);
            }
        });
        float far_clip = scene_camera.get_orthographic_far();
        label_row("Far", [&] {
            if (ImGui::DragFloat("##far", &far_clip, 1.f, 0.01f, 100000.f)) {
                scene_camera.set_orthographic_far(far_clip);
            }
        });
    } else {
        // FOV is already in degrees (see SceneCamera): no conversion here,
        // otherwise the slider would show 45 as 2578 deg.
        float fov = scene_camera.get_perspective_vertical_fov();
        label_row("FOV", [&] {
            if (ImGui::SliderFloat("##fov", &fov, 1.f, 179.f, "%.1f deg")) {
                scene_camera.set_perspective_vertical_fov(fov);
            }
        });
        float near_clip = scene_camera.get_perspective_near();
        label_row("Near", [&] {
            if (ImGui::DragFloat(
                    "##near", &near_clip, 0.01f, 0.001f, 10000.f)) {
                scene_camera.set_perspective_near(near_clip);
            }
        });
        float far_clip = scene_camera.get_perspective_far();
        label_row("Far", [&] {
            if (ImGui::DragFloat("##far", &far_clip, 1.f, 0.01f, 100000.f)) {
                scene_camera.set_perspective_far(far_clip);
            }
        });
    }

    label_row("Aspect", [&] {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%.3f (from viewport)",
                            scene_camera.get_aspect_ratio());
    });
    label_row("Fixed Aspect", [&] {
        ImGui::Checkbox("##fixed_aspect", &camera.fixed_aspect_ratio);
    });

    // The checkbox routes through make_primary() so turning one camera on turns
    // the others off, matching what the renderer expects.
    bool primary = camera.primary;
    label_row("Primary", [&] {
        if (ImGui::Checkbox("##primary", &primary)) {
            if (primary) {
                make_primary(context, entity);
            } else {
                camera.primary = false;
            }
        }
    });

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
        ImGui::Text(
            "m00: %.6f   m11: %.6f", projection[0][0], projection[1][1]);
        ImGui::Text(
            "m22: %.6f   m32: %.6f", projection[2][2], projection[3][2]);
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

    // Picking a script binds it (and destroys the previous instance).
    label_row("Script", [&] {
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
    });

    // An unbound script cannot run, so its toggle would only be misleading.
    ImGui::BeginDisabled(!script.is_bound());
    label_row("Enabled", [&] {
        ImGui::Checkbox("##enabled", &script.enabled);
        if (script.is_bound()) {
            ImGui::SameLine();
            if (ImGui::Button("Unbind")) {
                const auto &tag = entity.get_component<TagComponent>();
                IVX_INFO("Unbound script from '{}'", tag.value);
                script.unbind();
            }
        }
    });
    ImGui::EndDisabled();
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

    // Id/Tag is edited by the inspector header (the entity name), so it has no
    // section of its own and cannot be added/removed.
    registry.add<IdComponent>(
        "Uid",
        /*draw*/ {},
        {.removable = false, .addable = false, .draw_in_inspector = false});
    registry.add<TagComponent>(
        "Tag",
        /*draw*/ {},
        {.removable = false, .addable = false, .draw_in_inspector = false});
    registry.add<TransformComponent>(
        "Transform",
        draw_transform,
        {.removable = false, .addable = false, .draw_in_inspector = true});
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
