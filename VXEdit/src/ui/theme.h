#pragma once

#include <imgui.h>

namespace ivx::ui {

// Base font size in pixels (96 dpi reference). Text is rendered dynamically, so
// any size works; the scale below is what the UI-scale menu multiplies this by.
inline constexpr float kBaseFontSize = 17.f;
// Monospace is set a little smaller: the same nominal size looks larger in it.
inline constexpr float kBaseMonoFontSize = 16.f;
// Console and build output use the code face at this fraction of the monospace
// size: log lines are dense and a whole compiler log should fit on screen. This
// is the one number to change if the logs read too small or too large.
inline constexpr float kLogFontScale = 0.85f;

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

// Base size for log text (Console, Build output), already multiplied by the UI
// scale. Pass it to ImGui::PushFont(fonts.mono, ui::log_font_size()).
float log_font_size();

// The two colours widgets occasionally need beyond the stock ImGui ones: the
// single accent (Adobe blue, used for selection/focus everywhere) and the muted
// label gray. Exposed so panels never hardcode a hex value that drifts from the
// theme.
ImVec4 accent();
ImVec4 text_muted();

} // namespace ivx::ui
