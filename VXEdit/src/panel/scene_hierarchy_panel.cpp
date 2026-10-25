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
           ImGuiTreeNodeFlags_SpanAvailWidth |
           ImGuiTreeNodeFlags_DefaultOpen;
}
} // namespace

SceneHierarchyPanel::SceneHierarchyPanel() : Panel("Scene Hierarchy") {
}

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene)
    : Panel("Scene Hierarchy"), scene_(scene) {
}

void SceneHierarchyPanel::set_scene_context(const Ref<Scene> &scene) {
    scene_ = scene;
}

Entity SceneHierarchyPanel::get_selected() const {
    if (!scene_ || selected_ == entt::null ||
        !scene_->registry().valid(selected_)) {
        return {};
    }
    return Entity{selected_, scene_.get()};
}

void SceneHierarchyPanel::set_selected(Entity entity) {
    selected_ = entity ? entity.handle() : entt::null;
}

void SceneHierarchyPanel::on_imgui() {
    if (!scene_) {
        ImGui::TextUnformatted("No scene");
        return;
    }

    // The selection may have been destroyed from anywhere in the editor.
    if (selected_ != entt::null && !scene_->registry().valid(selected_)) {
        selected_ = entt::null;
    }

    draw_toolbar();
    ImGui::Separator();

    for (Entity root : scene_->get_root_entities()) {
        draw_entity_node(root);
    }

    // Clicking the empty area clears the selection.
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        selected_ = entt::null;
    }

    draw_empty_context_menu();
    flush_pending_delete();
}

void SceneHierarchyPanel::draw_toolbar() {
    if (ImGui::Button("+ Entity")) {
        set_selected(scene_->create_entity("Entity"));
    }
    ImGui::SameLine();
    ImGui::TextDisabled("| %zu root(s), %zu entity(ies)",
                        scene_->get_root_entities().size(),
                        scene_->registry().view<TagComponent>().size());
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
    if (selected_ == entity.handle()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool open = ImGui::TreeNodeEx("##node", flags, "%s", name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selected_ = entity.handle();
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
            const auto dragged = *static_cast<const entt::entity *>(
                payload->Data);
            if (scene_->registry().valid(dragged)) {
                scene_->set_parent(Entity{dragged, scene_.get()}, entity);
            }
        }
        ImGui::EndDragDropTarget();
    }

    draw_node_context_menu(entity);

    if (open) {
        for (auto child_handle : children) {
            if (!scene_->registry().valid(child_handle)) {
                continue; // stale entry, repaired by the next set_parent
            }
            draw_entity_node(Entity{child_handle, scene_.get()});
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneHierarchyPanel::draw_node_context_menu(Entity entity) {
    if (!ImGui::BeginPopupContextItem("##node_ctx")) {
        return;
    }

    selected_ = entity.handle();

    if (ImGui::MenuItem("Create Child")) {
        set_selected(scene_->create_child(entity, "Entity"));
    }
    if (ImGui::MenuItem("Add Sibling")) {
        Entity parent = entity.parent();
        set_selected(parent ? scene_->create_child(parent, "Entity")
                            : scene_->create_entity("Entity"));
    }
    if (ImGui::MenuItem("Unparent", nullptr, false, entity.has_parent())) {
        scene_->set_parent(entity, {});
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Delete", "Del")) {
        pending_delete_.push_back(entity.handle());
    }

    ImGui::EndPopup();
}

void SceneHierarchyPanel::draw_empty_context_menu() {
    if (!ImGui::BeginPopupContextWindow(
            "##empty_ctx",
            ImGuiPopupFlags_MouseButtonRight |
                ImGuiPopupFlags_NoOpenOverItems)) {
        return;
    }

    if (ImGui::MenuItem("Create Empty")) {
        set_selected(scene_->create_entity("Entity"));
    }
    ImGui::EndPopup();
}

void SceneHierarchyPanel::flush_pending_delete() {
    if (pending_delete_.empty()) {
        return;
    }

    for (auto handle : pending_delete_) {
        if (scene_->registry().valid(handle)) {
            scene_->destroy_entity(Entity{handle, scene_.get()});
        }
        if (selected_ == handle) {
            selected_ = entt::null;
        }
    }
    pending_delete_.clear();
}
} // namespace ivx
