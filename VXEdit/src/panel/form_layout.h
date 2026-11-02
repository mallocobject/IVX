#pragma once

#include "ui/theme.h"

#include <cmath>
#include <imgui.h>

namespace ivx {

// Unity-style rows: the label sits in a fixed left column and the widget fills
// the rest of the row. ImGui draws labels to the right of widgets, so each row
// draws its own label.
inline float label_column() {
    // Wide enough for the longest row label in use, scaled with the UI.
    return std::ceil(ImGui::CalcTextSize("Fixed Aspect").x) +
           14.f * ui::scale();
}

template <typename Draw>
void label_row(const char *label, Draw &&draw) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(label_column());
    ImGui::SetNextItemWidth(-FLT_MIN);
    draw();
}

} // namespace ivx
