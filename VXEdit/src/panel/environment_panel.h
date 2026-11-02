#pragma once

#include "editor_context.h"
#include "panel.h"

namespace ivx {

// Scene wide imaging settings: ambient light and exposure. Lights themselves
// are entities with a Light component, so they are edited in the Inspector like
// any other component.
class EnvironmentPanel : public Panel {
  public:
    explicit EnvironmentPanel(EditorContext &context);

  protected:
    void on_imgui() override;

  private:
    EditorContext &context_;
};

} // namespace ivx
