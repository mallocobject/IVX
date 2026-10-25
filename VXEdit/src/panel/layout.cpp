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
//   +----------------+----------------------------+----------------+
//   | Scene          | Viewport / Scripts         | Inspector      |
//   | Hierarchy      |                            | Camera         |
//   |                |                            |                |
//   +----------------+----------------------------+----------------+
//   | Console / Stats                                             |
//   +-------------------------------------------------------------+
void setup_default_layout(ImGuiID dockspace_id) {
    if (dockspace_id == 0) {
        return;
    }

    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID center = dockspace_id;
    const ImGuiID left = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Left, 0.22f, nullptr, &center);
    const ImGuiID right = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Right, 0.28f, nullptr, &center);
    // Shorter than before (0.30): the viewport needs the height far more than a
    // console tab strip does.
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Down, 0.20f, nullptr, &center);

    // Tab order inside a node: the last window docked is the one shown on startup,
    // so the viewport is docked after the Scripts panel.
    ImGui::DockBuilderDockWindow("Scene Hierarchy", left);

    // Scripts share the centre as a tab, Godot style: a script editor and the
    // scene view are alternatives, not neighbours.
    ImGui::DockBuilderDockWindow("Scripts", center);
    ImGui::DockBuilderDockWindow("Viewport", center);

    // The camera panel is about the scene, not about the selection, but it is a
    // property sheet like the inspector, so it belongs in the same column instead
    // of taking a slot in the short bottom strip.
    ImGui::DockBuilderDockWindow("Camera", right);
    ImGui::DockBuilderDockWindow("Inspector", right);

    // Only the console occupies the bottom strip. "Stats" is a floating overlay in
    // the top-right corner (StatsPanel::begin_window) and the status bar sits on the
    // main viewport's bottom edge (EditLayer::draw_status_bar), so neither of them
    // competes for a dock slot.
    ImGui::DockBuilderDockWindow("Console", bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace ivx
