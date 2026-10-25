#pragma once

#include "editor_context.h"
#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include "panel.h"
#include <ivx.h>
#include <vector>

namespace ivx {
class SceneHierarchyPanel : public Panel {
  public:
    explicit SceneHierarchyPanel(EditorContext &context);

    // Convenience wrappers over the shared selection.
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

    EditorContext &context_;
    std::vector<entt::entity> pending_delete_;
};
} // namespace ivx
