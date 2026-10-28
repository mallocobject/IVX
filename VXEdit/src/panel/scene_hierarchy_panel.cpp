#include "scene_hierarchy_panel.h"

#include "ivx/scene/components.h"

#include <algorithm>
#include <cstdint>
#include <imgui.h>
#include <string>

namespace ivx {
namespace {
constexpr const char *kDragPayload = "IVX_SCENE_ENTITY";

ImGuiTreeNodeFlags base_flags() {
    return ImGuiTreeNodeFlags_OpenOnArrow |
           ImGuiTreeNodeFlags_OpenOnDoubleClick |
           ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
}

// Size for a freshly created sprite, in world units.
//
// Unity spawns its default Square sprite at 1 unit in a 10-unit-tall 2D view,
// i.e. ~10% of the view height. This scene has no fixed unit scale (the demo
// camera is 720 units tall), so derive the same *proportion* from the primary
// camera instead of hardcoding a number that would be invisible or huge
// elsewhere.
float default_sprite_size(Scene &scene) {
    constexpr float kFallback = 100.f;
    for (auto [handle, camera] :
         scene.registry().view<CameraComponent>().each()) {
        if (!camera.primary) {
            continue;
        }
        if (camera.camera.get_projection_type() !=
            SceneCamera::ProjectionType::Orthographic) {
            break;
        }
        return std::clamp(
            camera.camera.get_orthographic_size() * 0.1f, 1.f, kFallback);
    }
    return kFallback;
}
} // namespace

SceneHierarchyPanel::SceneHierarchyPanel(EditorContext &context)
    : Panel("Scene Hierarchy"), context_(context) {
    set_default_size(ImVec2{320.f, 520.f});
}

Entity SceneHierarchyPanel::get_selected() const {
    return context_.selected_entity();
}

void SceneHierarchyPanel::set_selected(Entity entity) {
    context_.select(entity);
}

void SceneHierarchyPanel::on_imgui() {
    if (!context_.scene) {
        // The editor starts without a scene (File > New or File > Open).
        // Creating entities here would have nowhere to be saved, so the panel
        // only explains how to get one.
        ImGui::TextDisabled("No scene loaded.");
        ImGui::TextDisabled("Use File > New or File > Open.");
        return;
    }
    context_.validate_selection();

    draw_toolbar();
    ImGui::Separator();

    for (Entity root : context_.scene->get_root_entities()) {
        draw_entity_node(root);
    }

    // Clicking the empty area clears the selection.
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        context_.clear_selection();
    }

    draw_empty_context_menu();
    flush_pending_delete();
}

void SceneHierarchyPanel::draw_toolbar() {
    Scene &scene = *context_.scene;

    // Spawns a usable entity, not an invisible one: origin + identity transform
    // like Unity's "Create Empty", plus a white placeholder sprite because this
    // editor has no gizmo icons to show an entity that renders nothing. Any
    // component can be removed afterwards.
    if (ImGui::Button("+ Entity")) {
        set_selected(create_default_entity(scene));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "New entity at the origin (position 0, rotation 0, scale 1)\n"
            "with a %.0fx%.0f white sprite so it is visible in the viewport.",
            static_cast<double>(default_sprite_size(scene)),
            static_cast<double>(default_sprite_size(scene)));
    }

    ImGui::SameLine();
    // Kept short on purpose: the hierarchy dock is narrow, and a long label
    // gets clipped instead of being readable.
    ImGui::TextDisabled("| %zu", scene.registry().view<TagComponent>().size());
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%zu root(s), %zu entity(ies)",
                          scene.get_root_entities().size(),
                          scene.registry().view<TagComponent>().size());
    }
}

Entity SceneHierarchyPanel::create_default_entity(Scene &scene) {
    // Naming follows the usual "Entity" / "Entity (1)" / "Entity (2)" scheme,
    // so a new object never shows up as a duplicate of an existing one. Names
    // are not unique in the scene, so this asks for a free one explicitly.
    std::string name = "Entity";
    for (int suffix = 1; scene.find_by_name(name); ++suffix) {
        name = std::format("Entity ({})", suffix);
    }

    // Transform: Scene::create_entity() already gives the identity transform
    // every engine spawns with (position 0, rotation 0, scale 1). Only the
    // visual default is ours to choose.
    Entity entity = scene.create_entity(name);

    // Unity's default sprite tint is pure white and its default quad is a
    // textureless square, so a flat white quad is the closest equivalent with
    // no asset pipeline yet. Size comes from the camera (see
    // default_sprite_size) so it reads on screen.
    auto &sprite = entity.add_component<SpriteRendererComponent>();
    const float size = default_sprite_size(scene);
    sprite.size = glm::vec2{size, size};
    sprite.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
    return entity;
}

void SceneHierarchyPanel::draw_entity_node(Entity entity) {
    if (!entity) {
        return;
    }

    const auto &tag = entity.get_component<TagComponent>();
    const auto children = entity.children();

    ImGui::PushID(static_cast<int>(static_cast<std::uint32_t>(
        static_cast<entt::id_type>(entity.handle()))));

    ImGuiTreeNodeFlags flags = base_flags();
    if (context_.selected == entity.handle()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool open =
        ImGui::TreeNodeEx("##node", flags, "%s", tag.value.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context_.selected = entity.handle();
    }

    // Drag: reparent onto another node.
    if (ImGui::BeginDragDropSource()) {
        auto payload = entity.handle();
        ImGui::SetDragDropPayload(kDragPayload, &payload, sizeof(payload));
        ImGui::TextUnformatted(tag.value.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload(kDragPayload)) {
            const auto dragged =
                *static_cast<const entt::entity *>(payload->Data);
            if (context_.scene->registry().valid(dragged)) {
                context_.scene->set_parent(
                    Entity{dragged, context_.scene.get()}, entity);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // maybe delete children node
    draw_node_context_menu(entity);

    if (open) {
        for (auto child_handle : children) {
            if (!context_.scene->registry().valid(child_handle)) {
                continue; // stale entry, repaired by the next set_parent
            }
            draw_entity_node(Entity{child_handle, context_.scene.get()});
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneHierarchyPanel::draw_node_context_menu(Entity entity) {
    if (!ImGui::BeginPopupContextItem("##node_ctx")) {
        return;
    }

    Scene &scene = *context_.scene;
    context_.selected = entity.handle();

    if (ImGui::MenuItem("Create Child")) {
        set_selected(scene.create_child(entity, "Entity"));
    }
    if (ImGui::MenuItem("Add Sibling")) {
        Entity parent = entity.parent();
        set_selected(parent ? scene.create_child(parent, "Entity")
                            : scene.create_entity("Entity"));
    }
    if (ImGui::MenuItem("Unparent", nullptr, false, entity.has_parent())) {
        scene.set_parent(entity, {});
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Delete", "Del")) {
        pending_delete_.push_back(entity.handle());
    }

    ImGui::EndPopup();
}

void SceneHierarchyPanel::draw_empty_context_menu() {
    if (!ImGui::BeginPopupContextWindow("##empty_ctx",
                                        ImGuiPopupFlags_MouseButtonRight |
                                            ImGuiPopupFlags_NoOpenOverItems)) {
        return;
    }

    if (ImGui::MenuItem("Create Empty")) {
        // set_selected(context_.scene->create_entity("Entity"));
        set_selected(create_default_entity(*context_.scene.get()));
    }
    ImGui::EndPopup();
}

void SceneHierarchyPanel::flush_pending_delete() {
    if (pending_delete_.empty()) {
        return;
    }

    for (auto handle : pending_delete_) {
        if (context_.scene->registry().valid(handle)) {
            context_.scene->destroy_entity(
                Entity{handle, context_.scene.get()});
        }
        if (context_.selected == handle) {
            context_.clear_selection();
        }
    }
    pending_delete_.clear();
}
} // namespace ivx
