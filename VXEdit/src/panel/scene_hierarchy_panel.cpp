#include "scene_hierarchy_panel.h"
#include "ivx/scene/entity.h"
#include <cstdint>
#include <imgui.h>

namespace ivx {
SceneHierarchyPanel::SceneHierarchyPanel() : Panel("Scene Hierarchy") {
}

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene)
    : Panel("Scene Hierarchy"), scene_(scene) {
}

void SceneHierarchyPanel::set_scene_context(const Ref<Scene> &scene) {
    scene_ = scene;
}

void SceneHierarchyPanel::on_imgui() {
    if (!scene_) {
        return;
    }

    for (auto [handle, tag] : scene_->registry().view<TagComponent>().each()) {
        Entity entity{handle, scene_.get()};
        draw_entity_node(entity);
    }
}

void SceneHierarchyPanel::draw_entity_node(Entity entity) {
}
} // namespace ivx
