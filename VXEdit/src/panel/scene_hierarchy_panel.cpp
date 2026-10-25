#include "scene_hierarchy_panel.h"

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
        ImGui::TextUnformatted("No scene");
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

    if (ImGui::Button("+ Entity")) {
        set_selected(scene.create_entity("Entity"));
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

void SceneHierarchyPanel::draw_entity_node(Entity entity) {
    if (!entity) {
        return;
    }

    const auto *tag = entity.try_get_component<TagComponent>();
    const std::string name = tag != nullptr ? tag->tag : std::string{"<none>"};
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

    const bool open = ImGui::TreeNodeEx("##node", flags, "%s", name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context_.selected = entity.handle();
    }

    // Drag: reparent onto another node.
    if (ImGui::BeginDragDropSource()) {
        auto payload = entity.handle();
        ImGui::SetDragDropPayload(kDragPayload, &payload, sizeof(payload));
        ImGui::TextUnformatted(name.c_str());
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
        set_selected(context_.scene->create_entity("Entity"));
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
