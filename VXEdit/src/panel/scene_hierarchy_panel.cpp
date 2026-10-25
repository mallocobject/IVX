#include "scene_hierarchy_panel.h"
#include "ivx/scene/entity.h"
#include <cstdint>
#include <imgui.h>

namespace ivx {
SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene)
    : scene_(scene) {
}

void SceneHierarchyPanel::set_scene_context(const Ref<Scene> &scene) {
    scene_ = scene;
}

void SceneHierarchyPanel::on_render() {
    ImGui::Begin("Scene Hierarchy");

    for (auto [handle, tag] : scene_->registry().view<TagComponent>().each()) {
        Entity entity{handle, scene_.get()};
        draw_entity_node(entity);
    }

    ImGui::End();
}

void SceneHierarchyPanel::draw_entity_node(Entity entity) {
}
} // namespace ivx