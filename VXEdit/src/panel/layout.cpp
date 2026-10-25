#include "layout.h"

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder* lives in the internal API

namespace ivx {

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
        center, ImGuiDir_Right, 0.30f, nullptr, &center);
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(
        center, ImGuiDir_Down, 0.28f, nullptr, &center);

    ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Viewport", center);
    ImGui::DockBuilderDockWindow("Settings", bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace ivx
