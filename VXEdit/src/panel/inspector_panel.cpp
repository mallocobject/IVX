#include "inspector_panel.h"

#include "component_registry.h"
#include "ivx/core/logger.h"
#include "ivx/scene/components.h"

#include <imgui.h>
#include <string>

namespace ivx {

InspectorPanel::InspectorPanel(EditorContext &context)
    : Panel("Inspector"), context_(context) {
    set_default_size(ImVec2{380.f, 640.f});
}

void InspectorPanel::on_imgui() {
    context_.validate_selection();

    if (!context_.has_selection()) {
        ImGui::TextDisabled("Nothing selected");
        ImGui::Separator();
        ImGui::TextWrapped(
            "Select an entity in the Scene Hierarchy to edit its components.");
        return;
    }

    Entity entity = context_.selected_entity();
    draw_entity_header(entity);
    ImGui::Separator();
    draw_components(entity);
    ImGui::Separator();
    draw_add_component_menu(entity);

    flush_pending(entity);
}

void InspectorPanel::draw_entity_header(Entity entity) {
    const auto *tag = entity.try_get_component<TagComponent>();
    if (tag == nullptr) {
        return;
    }

    // Re-seed the edit buffer whenever the selection changes, so typing in the
    // field is not overwritten by the component value every frame.
    if (name_buffer_entity_ != entity.handle()) {
        name_buffer_entity_ = entity.handle();
        std::snprintf(
            name_buffer_, sizeof(name_buffer_), "%s", tag->value.c_str());
    }

    // The one and only name editor: TagComponent has no section of its own.
    ImGui::TextDisabled(
        "Entity %u",
        static_cast<uint32_t>(static_cast<entt::id_type>(entity.handle())));

    ImGui::SetNextItemWidth(-FLT_MIN);
    // Live editing (no EnterReturnsTrue): the hierarchy renames as you type.
    // The tag is just a display name, so writing it directly is all that is
    // needed.
    if (ImGui::InputText("##name", name_buffer_, sizeof(name_buffer_))) {
        if (auto *editable = entity.try_get_component<TagComponent>()) {
            editable->value = name_buffer_;
        }
    }
}

void InspectorPanel::draw_components(Entity entity) {
    const auto &entries = ComponentRegistry::get().entries();

    for (const ComponentEntry &entry : entries) {
        // Tag is edited by the header above, so it must not be drawn twice.
        if (!entry.flags.draw_in_inspector) {
            continue;
        }
        if (!entry.has || !entry.has(entity)) {
            continue;
        }

        ImGui::PushID(entry.name.c_str());

        const bool open = ImGui::CollapsingHeader(
            entry.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

        // Right-click on the header: destructive actions (context menus are
        // more reliable than popups anchored to a button, and they cannot be
        // clipped by the window edge).
        if (entry.flags.removable &&
            ImGui::BeginPopupContextItem("##entry_ctx")) {
            if (ImGui::MenuItem("Remove Component")) {
                pending_remove_.push_back(entry.name);
            }
            ImGui::EndPopup();
        }

        if (open && entry.draw) {
            entry.draw(entity, context_);
        }

        ImGui::PopID();
    }
}

void InspectorPanel::draw_add_component_menu(Entity entity) {
    const auto &entries = ComponentRegistry::get().entries();

    if (!ImGui::CollapsingHeader("Add Component")) {
        return;
    }

    bool any_available = false;
    for (const ComponentEntry &entry : entries) {
        // Not addable (e.g. Camera: created as an entity from the Camera panel)
        // or already present -> nothing to offer here.
        if (!entry.flags.addable) {
            continue;
        }
        if (entry.has && entry.has(entity)) {
            continue;
        }
        any_available = true;
        if (ImGui::Selectable(("+ " + entry.name).c_str())) {
            pending_add_.push_back(entry.name);
        }
    }
    if (!any_available) {
        ImGui::TextDisabled("Nothing left to add");
    }
}

void InspectorPanel::flush_pending(Entity entity) {
    if (pending_remove_.empty() && pending_add_.empty()) {
        return;
    }

    const auto &entries = ComponentRegistry::get().entries();

    // Removals first: adding and removing the same type in one frame would
    // otherwise depend on the order of the two lists.
    for (const std::string &name : pending_remove_) {
        for (const ComponentEntry &entry : entries) {
            if (entry.name == name && entry.remove) {
                entry.remove(entity);
                break;
            }
        }
    }
    for (const std::string &name : pending_add_) {
        for (const ComponentEntry &entry : entries) {
            if (entry.name == name && entry.add && !entry.has(entity)) {
                entry.add(entity);
                break;
            }
        }
    }

    pending_remove_.clear();
    pending_add_.clear();
}

} // namespace ivx
