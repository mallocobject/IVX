#include "stats_panel.h"

#include <imgui_internal.h> // FindWindowByName: the overlay hangs off the viewport

#include "ivx/core/application.h"
#include "ivx/core/logger.h"
#include "ivx/core/window.h"
#include "ivx/renderer/renderer2d.h"

#include <algorithm>
#include <imgui.h>

namespace ivx {
namespace {

constexpr ImU32 kGood =
    IM_COL32(110, 210, 130, 255); // comfortably inside budget
constexpr ImU32 kWarn = IM_COL32(235, 200, 90, 255); // near the 60 Hz budget
constexpr ImU32 kBad = IM_COL32(235, 110, 110, 255); // over budget

ImU32 budget_color(float ms) {
    if (ms > StatsPanel::kBudgetMs) {
        return kBad;
    }
    if (ms > StatsPanel::kBudgetMs * 0.6f) {
        return kWarn;
    }
    return kGood;
}

// "1.23k" style, keeps the renderer rows narrow.
void format_count(char *buffer, std::size_t size, float value) {
    if (value >= 1000.f) {
        std::snprintf(buffer, size, "%.1fk", value / 1000.f);
    } else {
        std::snprintf(buffer, size, "%.0f", value);
    }
}

} // namespace

StatsPanel::StatsPanel() : Panel("Stats") {
    set_default_size(ImVec2{560.f, 360.f});
    // Floating overlay: it must never take a dock slot, and it should not steal
    // the focus when it appears or when the layout is rebuilt.
    set_window_flags(ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav);
}

bool StatsPanel::begin_window() {
    // Anchored inside the viewport's top-right corner rather than the window's:
    // the right column belongs to the inspector, and covering it would hide the
    // panel the overlay is supposed to complement.
    ImVec2 anchor;
    if (const ImGuiWindow *viewport_window =
            ImGui::FindWindowByName("Viewport")) {
        // +32 skips the viewport's own tab bar, so the overlay starts below it.
        anchor = ImVec2{viewport_window->Pos.x + viewport_window->Size.x - 18.f,
                        viewport_window->Pos.y + 32.f};
    } else {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        anchor = ImVec2{viewport->WorkPos.x + viewport->WorkSize.x - 18.f,
                        viewport->WorkPos.y + 18.f};
    }

    // Pivot (1, 0) pins the right edge, so the overlay stays in the corner when
    // its size changes (e.g. the phase list grows).
    //
    // The position is forced until the anchor looks sane: the dock layout is
    // built over the first frames, and committing FirstUseEver to an anchor
    // measured before the viewport got its rect would pin the overlay
    // off-screen for the rest of the session.
    if (!placed_) {
        ImGui::SetNextWindowPos(anchor, ImGuiCond_Always, ImVec2{1.f, 0.f});
        const ImGuiViewport *main = ImGui::GetMainViewport();
        const bool inside_work_area =
            anchor.x > main->WorkPos.x + 120.f &&
            anchor.x < main->WorkPos.x + main->WorkSize.x - 10.f &&
            anchor.y > main->WorkPos.y &&
            anchor.y < main->WorkPos.y + main->WorkSize.y - 120.f;
        if (inside_work_area && ImGui::GetFrameCount() > 2) {
            placed_ = true;
        }
    }
    // Slightly transparent so the scene behind it stays visible.
    ImGui::SetNextWindowBgAlpha(0.88f);
    return Panel::begin_window();
}

void StatsPanel::end_window() {
    Panel::end_window();

    // The overlay floats on top of the viewport, so clicking the viewport would
    // bury it: keep it at the front of the window stack while it is open.
    if (ImGuiWindow *window = ImGui::FindWindowByName(name_.c_str())) {
        ImGui::BringWindowToDisplayFront(window);
    }
}

void StatsPanel::begin_frame(float delta_time) {
    // The phases collected during the previous frame are complete now.
    reported_ = std::move(current_);
    current_.clear();

    frame_ms_ = delta_time * 1000.f;

    constexpr float kAlpha = 0.1f;
    smoothed_ms_ += (frame_ms_ - smoothed_ms_) * kAlpha;

    history_[history_offset_] = frame_ms_;
    history_offset_ = (history_offset_ + 1) % kHistorySize;
    history_count_ = std::min(history_count_ + 1, kHistorySize);

    min_ms_ = history_[0];
    max_ms_ = history_[0];
    float sum = 0.f;
    for (int i = 0; i < history_count_; i++) {
        min_ms_ = std::min(min_ms_, history_[i]);
        max_ms_ = std::max(max_ms_, history_[i]);
        sum += history_[i];
    }
    avg_ms_ = history_count_ > 0 ? sum / static_cast<float>(history_count_)
                                 : frame_ms_;
}

void StatsPanel::submit(const char *phase, float milliseconds) {
    current_.push_back({phase, milliseconds});
}

void StatsPanel::on_imgui() {
    // Numbers left, graph right. The info column keeps a readable width, the
    // graph absorbs the rest.
    const float total_width = ImGui::GetContentRegionAvail().x;
    const float info_width = std::clamp(total_width * 0.5f, 220.f, 420.f);

    ImGui::BeginChild("##stats_info", ImVec2{info_width, 0.f});
    draw_info_column();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##stats_graph", ImVec2{0.f, 0.f});
    draw_graph_column();
    ImGui::EndChild();
}

void StatsPanel::draw_info_column() {
    const ImU32 color = budget_color(smoothed_ms_);
    const float fps = smoothed_ms_ > 0.f ? 1000.f / smoothed_ms_ : 0.f;

    // ---- headline ----
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("%.2f ms", smoothed_ms_);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::TextDisabled("(%.0f FPS)", fps);
    ImGui::SameLine(ImGui::GetContentRegionMax().x - 62.f);
    bool vsync = Application::get().get_window().is_v_sync();
    if (ImGui::Checkbox("VSync", &vsync)) {
        Application::get().get_window().set_v_sync(vsync);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Turn off to measure the actual frame cost\n"
            "(with VSync on the frame is capped to the refresh rate).");
    }

    ImGui::TextDisabled("budget %.1f ms  |  %.0f%% used",
                        kBudgetMs,
                        smoothed_ms_ > 0.f ? smoothed_ms_ / kBudgetMs * 100.f
                                           : 0.f);
    ImGui::TextDisabled("last %.2f  avg %.2f", frame_ms_, avg_ms_);
    ImGui::TextDisabled("min %.2f  max %.2f", min_ms_, max_ms_);

    // ---- phases ----
    ImGui::SeparatorText("Phases");

    float measured = 0.f;
    if (ImGui::BeginTable("##phases",
                          2,
                          ImGuiTableFlags_SizingStretchProp |
                              ImGuiTableFlags_NoSavedSettings)) {
        ImGui::TableSetupColumn(
            "phase", ImGuiTableColumnFlags_WidthFixed, 62.f);
        ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthStretch);

        auto row = [this](const char *name, float ms, ImU32 color) {
            const float share = frame_ms_ > 0.f ? ms / frame_ms_ * 100.f : 0.f;
            char label[48] = {};
            // The phase name already owns the first column, so the bar only
            // carries the numbers.
            std::snprintf(label, sizeof(label), "%.2f ms   %.1f%%", ms, share);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(name);

            ImGui::TableSetColumnIndex(1);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
            ImGui::ProgressBar(std::clamp(share / 100.f, 0.f, 1.f),
                               ImVec2{-FLT_MIN, 0.f},
                               label); // phase name is the first column, so
                                       // repeat only the numbers here
            ImGui::PopStyleColor();
        };

        for (const Phase &phase : reported_) {
            measured += phase.ms;
            row(phase.name.c_str(), phase.ms, IM_COL32(120, 160, 230, 255));
        }

        // Whatever is left is VSync wait / present / driver time, which no CPU
        // timer inside the layer can see.
        const float rest = std::max(0.f, frame_ms_ - measured);
        row("rest", rest, IM_COL32(130, 130, 145, 255));

        ImGui::EndTable();
    }

    // ---- renderer + imgui ----
    ImGui::SeparatorText("Renderer2D");
    const auto stats = Renderer2D::get_stats();
    char verts[16] = {};
    char indices[16] = {};
    format_count(verts,
                 sizeof(verts),
                 static_cast<float>(stats.get_total_vertex_count()));
    format_count(indices,
                 sizeof(indices),
                 static_cast<float>(stats.get_total_index_count()));
    ImGui::TextDisabled("draw calls %d   quads %d   verts %s   idx %s",
                        stats.num_draw_calls,
                        stats.num_quads,
                        verts,
                        indices);

    ImGui::SeparatorText("ImGui");
    if (const ImDrawData *draw_data = ImGui::GetDrawData()) {
        char lists[16] = {};
        char vtx[16] = {};
        format_count(
            lists, sizeof(lists), static_cast<float>(draw_data->CmdListsCount));
        format_count(
            vtx, sizeof(vtx), static_cast<float>(draw_data->TotalVtxCount));
        ImGui::TextDisabled("lists %s   verts %s   windows %d",
                            lists,
                            vtx,
                            ImGui::GetIO().MetricsRenderWindows);
    }
}

void StatsPanel::draw_graph_column() {
    // Scale: always keep the 60 Hz budget visible, otherwise fit the window.
    const float scale_max = std::max({kBudgetMs * 1.1f, max_ms_ * 1.1f, 1.f});

    ImGui::PushStyleColor(ImGuiCol_Text,
                          ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::Text("frame time (ms)  last %d", history_count_);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, budget_color(smoothed_ms_));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(22, 22, 26, 255));
    ImGui::PlotLines("##frame_time",
                     history_,
                     history_count_,
                     history_offset_,
                     nullptr,
                     0.f,
                     scale_max,
                     ImVec2{-FLT_MIN, -FLT_MIN});
    ImGui::PopStyleColor(2);

    // Budget reference line, drawn over the graph.
    const ImVec2 plot_min = ImGui::GetItemRectMin();
    const ImVec2 plot_max = ImGui::GetItemRectMax();
    const float height = plot_max.y - plot_min.y;
    if (height > 1.f) {
        const float y = plot_max.y - (kBudgetMs / scale_max) * height;
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(ImVec2{plot_min.x, y},
                           ImVec2{plot_max.x, y},
                           IM_COL32(235, 200, 90, 110),
                           1.f);
        char label[32] = {};
        std::snprintf(label, sizeof(label), "%.1f ms", kBudgetMs);
        draw_list->AddText(
            ImVec2{plot_min.x + 4.f, y - ImGui::GetTextLineHeight()},
            IM_COL32(235, 200, 90, 180),
            label);
    }
}

} // namespace ivx
