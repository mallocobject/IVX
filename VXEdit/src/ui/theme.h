#pragma once

#include <imgui.h>

namespace ivx::ui {

// Base font size in pixels (96 dpi reference). Text is rendered dynamically, so
// any size works; the scale below is what the UI-scale menu multiplies this by.
inline constexpr float kBaseFontSize = 17.f;
// Monospace is set a little smaller: the same nominal size looks larger in it.
inline constexpr float kBaseMonoFontSize = 16.f;

// Photoshop-style dark theme: neutral grays, square corners, tight rows, one
// blue accent used only for selection/focus, and a proportional UI font with a
// monospace one for code.
struct Fonts {
    ImFont *ui{nullptr};   // Segoe UI (falls back to the monospace face)
    ImFont *mono{nullptr}; // Consolas: code editor and console
};

// Applies colours + metrics and the persisted UI scale. Call once, before the
// first frame.
void apply_theme();

// Loads the two faces and installs `ui` as the default font. Returns references
// to both so panels can push the monospace font where it matters.
const Fonts &load_fonts();

// 1.0 = 100%. Rescales fonts and every spacing metric together, so the layout
// keeps its proportions. Persisted next to the editor assets.
void set_scale(float scale);
float scale();

// The two colours widgets occasionally need beyond the stock ImGui ones: the
// single accent (Adobe blue, used for selection/focus everywhere) and the muted
// label gray. Exposed so panels never hardcode a hex value that drifts from the
// theme.
ImVec4 accent();
ImVec4 text_muted();

} // namespace ivx::ui
