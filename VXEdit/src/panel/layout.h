#pragma once

#include <imgui.h>

namespace ivx {

// Builds the default editor docking layout (Hierarchy | Viewport | Inspector,
// Settings at the bottom of the viewport column). Call once after
// DockSpaceOverViewport() has returned the dockspace id, and again whenever the
// user asks for "Reset Layout".
void setup_default_layout(ImGuiID dockspace_id);

} // namespace ivx
