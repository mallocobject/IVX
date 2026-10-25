#include "stats_panel.h"

#include "ivx/core/application.h"
#include "ivx/core/window.h"
#include "ivx/renderer/renderer2d.h"

#include <algorithm>
#include <imgui.h>

namespace ivx {

StatsPanel::StatsPanel() : Panel("Stats") {
    set_default_size(ImVec2{360.f, 300.f});
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

    min_ms_ = frame_ms_;
    max_ms_ = frame_ms_;
    for (int i = 0; i < history_count_; i++) {
        min_ms_ = std::min(min_ms_, history_[i]);
        max_ms_ = std::max(max_ms_, history_[i]);
    }
}

void StatsPanel::submit(const char *phase, float milliseconds) {
    current_.push_back({phase, milliseconds});
}

void StatsPanel::on_imgui() {
    const float fps = smoothed_ms_ > 0.f ? 1000.f / smoothed_ms_ : 0.f;

    ImGui::Text("%.2f ms  (%.0f FPS)", smoothed_ms_, fps);
    ImGui::SameLine();
    ImGui::TextDisabled("| last %.2f  min %.2f  max %.2f", frame_ms_, min_ms_, max_ms_);

    // Measuring the real cost needs VSync off, otherwise the frame is capped and
    // most of the time is spent waiting for the display.
    bool vsync = Application::get().get_window().is_v_sync();
    ImGui::SameLine();
    if (ImGui::Checkbox("VSync", &vsync)) {
        Application::get().get_window().set_v_sync(vsync);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Turn off to measure the actual frame cost\n"
                          "(with VSync on the frame is capped to the refresh rate).");
    }

    // Frame time graph. The scale keeps 16.7 ms (60 Hz) visible even when the
    // frame is much faster, so a healthy frame is not a flat line.
    const float scale_max = std::max({16.7f, max_ms_ * 1.1f, 1.f});
    ImGui::PlotLines("##frame_time",
                     history_,
                     history_count_,
                     history_offset_,
                     nullptr,
                     0.f,
                     scale_max,
                     ImVec2{-FLT_MIN, 60.f});

    if (ImGui::TreeNodeEx("Phases", ImGuiTreeNodeFlags_DefaultOpen)) {
        float measured = 0.f;
        for (const Phase &phase : reported_) {
            measured += phase.ms;
            const float share = frame_ms_ > 0.f ? phase.ms / frame_ms_ * 100.f : 0.f;
            ImGui::Text("%-8s %6.2f ms  (%4.1f%%)", phase.name.c_str(), phase.ms, share);
            ImGui::SameLine();
            ImGui::ProgressBar(std::min(share / 100.f, 1.f),
                               ImVec2{-FLT_MIN, 0.f},
                               "");
        }

        // Whatever is left is VSync wait / present / driver time, which no CPU
        // timer inside the layer can see.
        const float rest = std::max(0.f, frame_ms_ - measured);
        ImGui::TextDisabled("rest     %6.2f ms  (vsync wait, present, driver)",
                            rest);
        ImGui::SameLine();
        ImGui::ProgressBar(frame_ms_ > 0.f ? rest / frame_ms_ : 0.f,
                           ImVec2{-FLT_MIN, 0.f},
                           "");
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Renderer2D", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto stats = Renderer2D::get_stats();
        ImGui::Text("draw calls %d   quads %d", stats.num_draw_calls, stats.num_quads);
        ImGui::Text("vertices %d   indices %d",
                    stats.get_total_vertex_count(),
                    stats.get_total_index_count());
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("ImGui", ImGuiTreeNodeFlags_DefaultOpen)) {
        const ImDrawData *draw_data = ImGui::GetDrawData();
        if (draw_data != nullptr) {
            ImGui::Text("draw lists %d   vertices %d",
                        draw_data->CmdListsCount,
                        draw_data->TotalVtxCount);
            ImGui::Text("windows %d", ImGui::GetIO().MetricsRenderWindows);
        }
        ImGui::TreePop();
    }
}

} // namespace ivx
