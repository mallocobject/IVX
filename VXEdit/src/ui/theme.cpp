#include "ui/theme.h"

#include "ivx/core/logger.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace ivx::ui {
namespace {

// Photoshop's dark UI is a small ladder of neutral grays plus a single blue
// accent. Keeping the ladder explicit makes it obvious which step a widget
// should use. 0xRRGGBB -> ImVec4. ImU32/IM_COL32 is ABGR, so the channels must
// be reordered: passing a plain 0xRRGGBB to ColorConvertU32ToFloat4() yields
// alpha 0, i.e. an invisible UI.
ImVec4 color(ImU32 rgb_value, float alpha = 1.f) {
    return ImVec4{((rgb_value >> 16) & 0xFF) / 255.f,
                  ((rgb_value >> 8) & 0xFF) / 255.f,
                  (rgb_value & 0xFF) / 255.f,
                  alpha};
}

constexpr ImU32 kChrome = 0x262626;  // menu bar, title bars, scrollbar track
constexpr ImU32 kPanel = 0x323232;   // window / child background
constexpr ImU32 kControl = 0x3C3C3C; // buttons, input fields
constexpr ImU32 kControlHover = 0x484848;
constexpr ImU32 kControlActive = 0x535353;
constexpr ImU32 kRow = 0x3C3C3C; // tree rows, selectables, collapsing headers
constexpr ImU32 kRowHover = 0x464646;
constexpr ImU32 kRowActive = 0x505050;
constexpr ImU32 kSeparator = 0x454545;
constexpr ImU32 kBorder = 0x1C1C1C;
constexpr ImU32 kText = 0xD6D6D6;
constexpr ImU32 kTextDisabled = 0x7C7C7C;
constexpr ImU32 kAccent = 0x1473E6; // Adobe blue

constexpr ImU32 kTabUnselected = 0x292929;
constexpr ImU32 kTabHovered = 0x353535;
constexpr ImU32 kTabSelected = 0x323232; // merges into the panel body below
constexpr ImU32 kTabDimmed = 0x242424;
constexpr ImU32 kTabDimmedSelected = 0x2E2E2E;

ImVec4 with_alpha(ImU32 rgb_value, float alpha) {
    return color(rgb_value, alpha);
}

std::filesystem::path windows_font(const char *file) {
    const char *windir = std::getenv("WINDIR");
    if (windir == nullptr) {
        return {};
    }
    return std::filesystem::path{windir} / "Fonts" / file;
}

// Style at 100%: set_scale() rescales from this copy so repeated changes never
// accumulate drift.
ImGuiStyle base_style;

std::filesystem::path scale_file() {
    // Next to the editor assets (like layout.ini) so it survives being launched
    // from anywhere; gitignored because it is a per-machine preference.
    return std::filesystem::path{LAYOUT}.parent_path() / "ui_scale.txt";
}

float load_scale() {
    std::ifstream file{scale_file()};
    float value = 1.f;
    if (file >> value && value >= 0.8f && value <= 2.f) {
        return value;
    }
    return 1.f;
}

void save_scale(float value) {
    std::ofstream file{scale_file(), std::ios::trunc};
    if (file) {
        file << value;
    }
}

} // namespace

void apply_theme() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // ---- surfaces -----------------------------------------------------------
    colors[ImGuiCol_WindowBg] = color(kPanel);
    colors[ImGuiCol_ChildBg] = color(kPanel);
    colors[ImGuiCol_PopupBg] = color(0x353535);
    colors[ImGuiCol_MenuBarBg] = color(kChrome);
    colors[ImGuiCol_TitleBg] = color(kChrome);
    colors[ImGuiCol_TitleBgActive] = color(0x3A3A3A);
    colors[ImGuiCol_TitleBgCollapsed] = color(kChrome);
    colors[ImGuiCol_Border] = color(kBorder);
    colors[ImGuiCol_BorderShadow] = ImVec4{0.f, 0.f, 0.f, 0.f};
    colors[ImGuiCol_DockingEmptyBg] = color(0x1D1D1D);

    // ---- text ---------------------------------------------------------------
    colors[ImGuiCol_Text] = color(kText);
    colors[ImGuiCol_TextDisabled] = color(kTextDisabled);
    colors[ImGuiCol_TextSelectedBg] = with_alpha(kAccent, 0.55f);
    colors[ImGuiCol_InputTextCursor] = color(0xDCDCDC);

    // ---- controls -----------------------------------------------------------
    // Fields are inset (darker than the panel), buttons are raised: that
    // contrast is what makes Photoshop's controls readable without borders.
    colors[ImGuiCol_FrameBg] = color(0x2A2A2A);
    colors[ImGuiCol_FrameBgHovered] = color(0x333333);
    colors[ImGuiCol_FrameBgActive] = color(0x3C3C3C);
    colors[ImGuiCol_Button] = color(kControl);
    colors[ImGuiCol_ButtonHovered] = color(kControlHover);
    colors[ImGuiCol_ButtonActive] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_CheckMark] = color(0xFFFFFF);
    colors[ImGuiCol_CheckboxSelectedBg] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_SliderGrab] = color(0x6F6F6F);
    colors[ImGuiCol_SliderGrabActive] = with_alpha(kAccent, 1.f);

    // ---- rows (tree, selectable, collapsing header) -------------------------
    colors[ImGuiCol_Header] = color(kRow);
    colors[ImGuiCol_HeaderHovered] = color(kRowHover);
    colors[ImGuiCol_HeaderActive] = color(kRowActive);

    // ---- tabs ---------------------------------------------------------------
    colors[ImGuiCol_Tab] = color(kTabUnselected);
    colors[ImGuiCol_TabHovered] = color(kTabHovered);
    colors[ImGuiCol_TabSelected] = color(kTabSelected);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4{0.f, 0.f, 0.f, 0.f};
    colors[ImGuiCol_TabDimmed] = color(kTabDimmed);
    colors[ImGuiCol_TabDimmedSelected] = color(kTabDimmedSelected);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4{0.f, 0.f, 0.f, 0.f};

    // ---- separators, scrollbars, grips -------------------------------------
    colors[ImGuiCol_Separator] = color(kSeparator);
    colors[ImGuiCol_SeparatorHovered] = with_alpha(kAccent, 0.7f);
    colors[ImGuiCol_SeparatorActive] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_ScrollbarBg] = color(0x252525);
    colors[ImGuiCol_ScrollbarGrab] = color(0x4A4A4A);
    colors[ImGuiCol_ScrollbarGrabHovered] = color(0x585858);
    colors[ImGuiCol_ScrollbarGrabActive] = color(0x686868);
    colors[ImGuiCol_ResizeGrip] = with_alpha(kAccent, 0.18f);
    colors[ImGuiCol_ResizeGripHovered] = with_alpha(kAccent, 0.35f);
    colors[ImGuiCol_ResizeGripActive] = with_alpha(kAccent, 0.55f);

    // ---- plots, tables, docking --------------------------------------------
    colors[ImGuiCol_PlotLines] = color(0x4C9FE8);
    colors[ImGuiCol_PlotLinesHovered] = color(0x7CC0FF);
    colors[ImGuiCol_PlotHistogram] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_PlotHistogramHovered] = color(0x4C9FE8);
    colors[ImGuiCol_TableHeaderBg] = color(0x3A3A3A);
    colors[ImGuiCol_TableBorderStrong] = color(0x454545);
    colors[ImGuiCol_TableBorderLight] = color(0x3E3E3E);
    colors[ImGuiCol_TableRowBg] = ImVec4{0.f, 0.f, 0.f, 0.f};
    colors[ImGuiCol_TableRowBgAlt] = color(0xFFFFFF, 0.02f);
    colors[ImGuiCol_DockingPreview] = with_alpha(kAccent, 0.35f);
    colors[ImGuiCol_TextLink] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_TreeLines] = color(0x4A4A4A);
    colors[ImGuiCol_DragDropTarget] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_DragDropTargetBg] = with_alpha(kAccent, 0.25f);
    colors[ImGuiCol_UnsavedMarker] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_NavCursor] = with_alpha(kAccent, 1.f);
    colors[ImGuiCol_NavWindowingHighlight] = with_alpha(0xFFFFFF, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = with_alpha(0x000000, 0.35f);
    colors[ImGuiCol_ModalWindowDimBg] = with_alpha(0x000000, 0.45f);

    // ---- metrics: square corners, tight rows ------------------------------
    style.WindowPadding = ImVec2{8.f, 6.f};
    style.FramePadding = ImVec2{7.f, 3.f};
    style.CellPadding = ImVec2{4.f, 2.f};
    style.ItemSpacing = ImVec2{7.f, 4.f};
    style.ItemInnerSpacing = ImVec2{5.f, 3.f};
    style.TouchExtraPadding = ImVec2{0.f, 0.f};
    style.IndentSpacing = 16.f;
    style.ColumnsMinSpacing = 6.f;
    style.ScrollbarSize = 12.f;
    style.GrabMinSize = 10.f;

    style.WindowBorderSize = 1.f;
    style.ChildBorderSize = 1.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 0.f;
    style.TabBarBorderSize = 1.f;
    style.TabBarOverlineSize = 0.f;
    style.SeparatorTextBorderSize = 1.f;
    style.DockingSeparatorSize = 2.f;

    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 2.f;
    style.PopupRounding = 2.f;
    style.ScrollbarRounding = 0.f;
    style.GrabRounding = 2.f;
    style.TabRounding = 0.f;

    style.WindowTitleAlign = ImVec2{0.f, 0.5f};
    style.WindowMenuButtonPosition =
        ImGuiDir_None; // Photoshop panels have none
    style.ButtonTextAlign = ImVec2{0.5f, 0.5f};
    style.SelectableTextAlign = ImVec2{0.f, 0.5f};
    style.SeparatorTextAlign = ImVec2{0.f, 0.5f};
    style.SeparatorTextPadding = ImVec2{12.f, 3.f};
    style.LogSliderDeadzone = 4.f;

    style.FontSizeBase = kBaseFontSize;

    // Remember the 100% layout: set_scale() always rescales from this copy, so
    // switching back and forth never accumulates rounding drift.
    base_style = style;
    set_scale(load_scale());
}

float &current_scale() {
    static float value = 1.f;
    return value;
}

float scale() {
    return current_scale();
}

void set_scale(float value) {
    value = std::clamp(value, 0.8f, 2.f);
    current_scale() = value;

    ImGuiStyle &style = ImGui::GetStyle();
    style = base_style;
    // One multiplier for fonts, one for every spacing metric: text and paddings
    // grow together, so the layout keeps its proportions instead of just
    // getting cramped.
    style.FontSizeBase = kBaseFontSize * value;
    style.ScaleAllSizes(value);

    if (value != load_scale()) {
        save_scale(value);
    }
}

const Fonts &load_fonts() {
    static Fonts fonts;
    if (fonts.ui != nullptr) {
        return fonts;
    }

    ImGuiIO &io = ImGui::GetIO();

    // Both faces are loaded once at the base size; ImGui bakes whatever size
    // the style asks for at draw time, so the UI scale needs no reload.
    ImFontConfig mono_config;
    fonts.mono = io.Fonts->AddFontFromFileTTF(
        FONTS "/consola.ttf", kBaseMonoFontSize, &mono_config);
    if (fonts.mono == nullptr) {
        IVX_ERROR("Failed to load Consolas; using the built-in font");
        fonts.mono = io.Fonts->AddFontDefault();
    }

    // Proportional UI face. Segoe UI ships with Windows; the editor only reads
    // it, it is never copied into the repository.
    ImFontConfig ui_config;
    ui_config.OversampleH = 2; // proportional glyphs benefit from horizontal AA
    const std::filesystem::path segoe = windows_font("segoeui.ttf");
    if (!segoe.empty() && std::filesystem::exists(segoe)) {
        fonts.ui = io.Fonts->AddFontFromFileTTF(
            segoe.string().c_str(), kBaseFontSize, &ui_config);
    }
    if (fonts.ui == nullptr) {
        IVX_WARN("Segoe UI not found, falling back to the monospace face for "
                 "the UI");
        fonts.ui = fonts.mono;
    }

    io.FontDefault = fonts.ui;
    return fonts;
}

ImVec4 accent() {
    return color(kAccent);
}

ImVec4 text_muted() {
    return color(kTextDisabled);
}

float log_font_size() {
    // Style::FontSizeBase is the UI base size times the scale; log text uses
    // the smaller monospace base at kLogFontScale of it.
    return kBaseMonoFontSize * current_scale() * kLogFontScale;
}

} // namespace ivx::ui
