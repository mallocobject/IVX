#include "stats_panel.h"

#include <imgui_internal.h> // FindWindowByName: anchor at the viewport`s corner

#include "ivx/core/application.h"
#include "ivx/core/window.h"
#include "ivx/render/renderer2d.h"
#include "ivx/render/renderer3d.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <imgui.h>

namespace ivx {
namespace {

constexpr ImU32 kGood =
    IM_COL32(110, 210, 130, 255); // comfortably inside budget
constexpr ImU32 kWarn = IM_COL32(235, 200, 90, 255); // near the 60 Hz budget
constexpr ImU32 kBad = IM_COL32(235, 110, 110, 255); // over budget
constexpr ImU32 kPhase = IM_COL32(120, 160, 230, 255);
constexpr ImU32 kRest = IM_COL32(130, 130, 145, 255);

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
    // Sized in begin_window (it has to follow the UI scale), so the base class
    // must not apply its own first-use size.
    set_default_size(ImVec2{0.f, 0.f});
    // A window of its own: never take a dock slot, and do not steal focus when
    // it appears or when the layout is rebuilt.
    set_window_flags(ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav);
}

bool StatsPanel::begin_window() {
    const float current_scale = ui::scale();
    const ImVec2 size{520.f * current_scale, 400.f * current_scale};

    // The overlay is sized in pixels, so a UI-scale change has to re-size it or
    // the content overflows. Manual resizes still stick: the size is only
    // re-applied when the scale actually changed.
    if (!(std::abs(sized_for_scale_ - current_scale) < 0.001f)) {
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        sized_for_scale_ = current_scale;
    }

    // Slightly transparent so whatever is behind it stays readable.
    ImGui::SetNextWindowBgAlpha(0.94f);

    const bool open = Panel::begin_window();
    if (!open) {
        return false;
    }

    // Only the very first placement is forced, and only until the main window
    // has its real rect (the dock layout is built over the first frames).
    // Afterwards the window is left alone so a drag sticks, which is also what
    // lets it leave the main window and become an OS window of its own.
    if (!placed_) {
        const ImVec2 target = detached_position(size);
        ImGui::SetWindowPos(target, ImGuiCond_Always);
        if (ImGui::GetFrameCount() > 2) {
            placed_ = true;
        }
    }

    return true;
}

ImVec2 StatsPanel::detached_position(ImVec2 size) {
    const ImGuiViewport *main = ImGui::GetMainViewport();

    // Preferred spot: just right of the main window, which is exactly what
    // makes this an OS window of its own once multi-viewport is on. Falls back
    // to the main window's bottom-right corner when no monitor has room for it.
    ImVec2 target{main->Pos.x + main->Size.x + 12.f, main->Pos.y + 48.f};
    for (const ImGuiPlatformMonitor &monitor :
         ImGui::GetPlatformIO().Monitors) {
        const ImVec2 limit{monitor.WorkPos.x + monitor.WorkSize.x,
                           monitor.WorkPos.y + monitor.WorkSize.y};
        if (target.x >= monitor.WorkPos.x && target.y >= monitor.WorkPos.y &&
            target.x + size.x <= limit.x && target.y + size.y <= limit.y) {
            return target;
        }
    }

    // No monitor room: sit in the viewport`s top-right corner, below its tab
    // bar, which keeps the console and the inspector visible.
    if (const ImGuiWindow *viewport = ImGui::FindWindowByName("Viewport")) {
        return ImVec2{viewport->Pos.x + viewport->Size.x - size.x - 16.f,
                      viewport->Pos.y + 36.f};
    }

    return ImVec2{main->Pos.x + main->Size.x - size.x - 24.f,
                  main->Pos.y + 48.f};
}

void StatsPanel::begin_frame(float delta_time) {
    // The phases collected during the previous frame are complete now.
    reported_ = std::move(current_);
    current_.clear();

    // The very first delta covers start-up, not a frame: it would own the max,
    // the average and the graph scale for the rest of the session.
    if (!started_) {
        started_ = true;
        return;
    }

    // Clamping also keeps a hitch (a file dialog, a rebuild) from flattening
    // the graph.
    frame_ms_ = std::clamp(delta_time * 1000.f, 0.f, 1000.f);

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
    draw_headline();
    ImGui::Spacing();

    // Numbers left, graph right: the info column keeps a readable width and the
    // graph absorbs whatever is left.
    const float total_width = ImGui::GetContentRegionAvail().x;
    const float info_width = std::clamp(total_width * 0.52f, 220.f, 420.f);

    ImGui::BeginChild("##stats_info", ImVec2{info_width, 0.f});
    draw_info_column();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##stats_graph", ImVec2{0.f, 0.f});
    draw_graph_column();
    ImGui::EndChild();
}

void StatsPanel::draw_headline() {
    const ImU32 color = budget_color(smoothed_ms_);
    const float fps = smoothed_ms_ > 0.f ? 1000.f / smoothed_ms_ : 0.f;
    const float used = smoothed_ms_ > 0.f ? smoothed_ms_ / kBudgetMs : 0.f;

    const ImVec4 accent = ImGui::ColorConvertU32ToFloat4(color);

    // A tinted plate behind the numbers, so the panel reads at a glance instead
    // of being another wall of grey text.
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImVec4{accent.x, accent.y, accent.z, 0.10f});
    ImGui::BeginChild("##stats_headline",
                      ImVec2{0.f, 0.f},
                      ImGuiChildFlags_AutoResizeY |
                          ImGuiChildFlags_AlwaysUseWindowPadding);

    ImGui::PushStyleColor(ImGuiCol_Text, accent);
    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("%.0f", fps);
    ImGui::SetWindowFontScale(1.f);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::TextDisabled("FPS");

    ImGui::SameLine(ImGui::GetContentRegionMax().x - 74.f);
    bool vsync = Application::get().get_window().is_v_sync();
    if (ImGui::Checkbox("VSync", &vsync)) {
        Application::get().get_window().set_v_sync(vsync);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Turn off to measure the actual frame cost\n"
                          "(with VSync on the frame is capped to the refresh "
                          "rate).");
    }

    // Budget meter: the frame against 16.7 ms, in the headline's accent colour.
    char label[48] = {};
    std::snprintf(
        label, sizeof(label), "%.2f ms   %.0f%%", smoothed_ms_, used * 100.f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, accent);
    ImGui::ProgressBar(
        std::clamp(used, 0.f, 1.f), ImVec2{-FLT_MIN, 0.f}, label);
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void StatsPanel::draw_info_column() {
    ImGui::TextDisabled("budget %.1f ms", kBudgetMs);
    ImGui::TextDisabled("last %.2f   avg %.2f   min %.2f   max %.2f",
                        frame_ms_,
                        avg_ms_,
                        min_ms_,
                        max_ms_);

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
            std::snprintf(label, sizeof(label), "%.2f ms   %.1f%%", ms, share);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(name);

            ImGui::TableSetColumnIndex(1);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
            ImGui::ProgressBar(std::clamp(share / 100.f, 0.f, 1.f),
                               ImVec2{-FLT_MIN, 0.f},
                               label);
            ImGui::PopStyleColor();
        };

        for (const Phase &phase : reported_) {
            measured += phase.ms;
            row(phase.name.c_str(), phase.ms, kPhase);
        }

        // Whatever is left is VSync wait / present / driver time, which no CPU
        // timer inside the layer can see.
        const float rest = std::max(0.f, frame_ms_ - measured);
        row("rest", rest, kRest);

        ImGui::EndTable();
    }

    // ---- renderers ----
    ImGui::SeparatorText("Renderers");

    const auto &stats3d = Renderer3D::get_stats();
    char triangles[16] = {};
    format_count(
        triangles, sizeof(triangles), static_cast<float>(stats3d.triangles));
    ImGui::TextDisabled(
        "3D   draws %d   tris %s", stats3d.draw_calls, triangles);

    const auto stats2d = Renderer2D::get_stats();
    char verts[16] = {};
    char indices[16] = {};
    format_count(verts,
                 sizeof(verts),
                 static_cast<float>(stats2d.get_total_vertex_count()));
    format_count(indices,
                 sizeof(indices),
                 static_cast<float>(stats2d.get_total_index_count()));
    ImGui::TextDisabled("2D   draws %d   quads %d   verts %s   idx %s",
                        stats2d.num_draw_calls,
                        stats2d.num_quads,
                        verts,
                        indices);

    // ---- imgui ----
    if (const ImDrawData *draw_data = ImGui::GetDrawData()) {
        char lists[16] = {};
        char vtx[16] = {};
        format_count(
            lists, sizeof(lists), static_cast<float>(draw_data->CmdListsCount));
        format_count(
            vtx, sizeof(vtx), static_cast<float>(draw_data->TotalVtxCount));
        ImGui::TextDisabled("UI   lists %s   verts %s   windows %d",
                            lists,
                            vtx,
                            ImGui::GetIO().MetricsRenderWindows);
    }
}

void StatsPanel::draw_graph_column() {
    // Scale: always keep the 60 Hz budget visible, otherwise fit the window.
    const float scale_max = std::max({kBudgetMs * 1.1f, max_ms_ * 1.1f, 1.f});

    ImGui::TextDisabled("frame time (ms)   %d samples", history_count_);

    ImGui::PushStyleColor(ImGuiCol_PlotLines, budget_color(smoothed_ms_));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(22, 22, 26, 255));
    // While the buffer is still filling it is not a ring yet: offset 0 and the
    // count so far, otherwise PlotLines wraps past the end into zeros.
    const int offset = history_count_ < kHistorySize ? 0 : history_offset_;
    ImGui::PlotLines("##frame_time",
                     history_,
                     history_count_,
                     offset,
                     nullptr,
                     0.f,
                     scale_max,
                     ImVec2{-FLT_MIN, -FLT_MIN});
    ImGui::PopStyleColor(2);

    // Budget and average reference lines, drawn over the graph.
    const ImVec2 plot_min = ImGui::GetItemRectMin();
    const ImVec2 plot_max = ImGui::GetItemRectMax();
    const float height = plot_max.y - plot_min.y;
    if (height <= 1.f) {
        return;
    }

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    const float budget_y = plot_max.y - (kBudgetMs / scale_max) * height;
    draw_list->AddLine(ImVec2{plot_min.x, budget_y},
                       ImVec2{plot_max.x, budget_y},
                       IM_COL32(235, 200, 90, 110),
                       1.f);
    char label[32] = {};
    std::snprintf(label, sizeof(label), "%.1f ms", kBudgetMs);
    draw_list->AddText(
        ImVec2{plot_min.x + 4.f, budget_y - ImGui::GetTextLineHeight()},
        IM_COL32(235, 200, 90, 180),
        label);

    if (avg_ms_ > 0.f) {
        const float avg_y = plot_max.y - (avg_ms_ / scale_max) * height;
        draw_list->AddLine(ImVec2{plot_min.x, avg_y},
                           ImVec2{plot_max.x, avg_y},
                           IM_COL32(150, 150, 165, 90),
                           1.f);
    }
}

} // namespace ivx
