#include "layout.h"

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder* lives in the internal API

namespace ivx {

// Default dock layout. Two rules drive it:
//   * the viewport gets as much room as possible, so the panels around it stay
//     narrow and the bottom strip is short;
//   * panels are grouped by what they are about: scene contents on the left,
//     properties of the selection on the right, running output at the bottom.
//
//   +----------------+----------------------------------+----------------+
//   | Scene          |                                  | Inspector      |
//   | Hierarchy      | Viewport                         |                |
//   |                |                                  |                |
//   +                +----------------------------------+                +
//   |                |  Console                         |                |
//   +--------------------------------------------------------------------+
//
// The stats overlay floats above the viewport (StatsPanel::begin_window) and
// the status bar sits on the main viewport's bottom edge
// (EditLayer::draw_status_bar), so neither competes for a dock slot.
void setup_default_layout(ImGuiID dockspace_id) {
    if (dockspace_id == 0) {
        return;
    }

    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID center = dockspace_id;
    // The side columns are narrower than the usual 0.22/0.28: a hierarchy row
    // only needs a name and the inspector uses about a third of the screen,
    // while every pixel the centre loses is scene the user cannot see.
    const ImGuiID left = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Left, 0.18f, nullptr, &center);
    const ImGuiID right = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Right, 0.26f, nullptr, &center);
    // Short: the viewport needs the height far more than a log tab strip does.
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Down, 0.20f, nullptr, &center);

    ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
    ImGui::DockBuilderDockWindow("Viewport", center);
    // The inspector owns the right column on its own: a camera is edited there
    // like any other component, so there is no camera panel to sit next to it.
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Console", bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace ivx
