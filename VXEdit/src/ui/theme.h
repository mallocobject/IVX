#pragma once

#include <imgui.h>

namespace ivx::ui {

// Photoshop-style dark theme: neutral grays, square corners, tight rows, one
// blue accent used only for selection/focus, and a proportional UI font with a
// monospace one for code.
struct Fonts {
    ImFont *ui{nullptr};   // Segoe UI (falls back to the monospace face)
    ImFont *mono{nullptr}; // Consolas: code editor and console
};

// Applies colours + metrics. Call once, before the first frame.
void apply_theme();

// Loads the two faces and installs `ui` as the default font. Returns references
// to both so panels can push the monospace font where it matters.
const Fonts &load_fonts();

} // namespace ivx::ui
