#pragma once

#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include "panel.h"
#include <ivx.h>
#include <vector>

namespace ivx {
class SceneHierarchyPanel : public Panel {
  public:
    SceneHierarchyPanel();
    explicit SceneHierarchyPanel(const Ref<Scene> &scene);

    void set_scene_context(const Ref<Scene> &scene);

    Ref<Scene> get_scene() const {
        return scene_;
    }
    // Validated against the registry, so it is safe to call every frame.
    Entity get_selected() const;
    void set_selected(Entity entity);

  protected:
    void on_imgui() override;

  private:
    void draw_toolbar();
    void draw_entity_node(Entity entity);
    void draw_node_context_menu(Entity entity);
    void draw_empty_context_menu();
    void flush_pending_delete();

    Ref<Scene> scene_;
    entt::entity selected_{entt::null};
    std::vector<entt::entity> pending_delete_;
};
} // namespace ivx
