#pragma once

#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include <ivx.h>

namespace ivx {
class SceneHierarchyPanel {
  public:
    SceneHierarchyPanel() = default;
    SceneHierarchyPanel(const Ref<Scene> &scene);
    void set_scene_context(const Ref<Scene> &scene);
    void on_render();

  private:
    void draw_entity_node(Entity entity);

    Ref<Scene> scene_;
    Entity selected_entity_;
};
} // namespace ivx