#pragma once

#include "editor_context.h"
#include "panel.h"

#include <vector>

namespace ivx {

// Draws the components of the currently selected entity and lets the user add
// or remove them. The set of components comes from ComponentRegistry, so a new
// component type only needs one registration.
class InspectorPanel : public Panel {
  public:
    explicit InspectorPanel(EditorContext &context);

  protected:
    void on_imgui() override;

  private:
    void draw_entity_header(Entity entity);
    void draw_components(Entity entity);
    void draw_add_component_menu(Entity entity);
    void flush_pending(Entity entity);

    EditorContext &context_;
    std::vector<std::string> pending_add_;
    std::vector<std::string> pending_remove_;
    char name_buffer_[256]{};
    entt::entity name_buffer_entity_{entt::null};
};

} // namespace ivx
