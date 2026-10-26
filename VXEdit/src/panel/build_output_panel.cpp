#include "build_output_panel.h"

#include "ivx/core/logger.h"
#include "scripts/script_host.h"
#include "ui/theme.h"

#include <imgui.h>

#include <cstdio>

namespace ivx {
BuildOutputPanel::BuildOutputPanel(ScriptHost &host)
    : Panel("Build"), host_(host) {
    set_default_size(ImVec2{800.f, 220.f});
}

bool BuildOutputPanel::begin_window() {
    // The tab has to be focused *before* Begin() runs, which is why the idle ->
    // building edge is handled here and not in on_imgui(): IDEs switch the
    // Output window to "Build" when a build starts, and a log nobody is looking
    // at is not much use.
    const bool building = host_.is_building();
    if (building && !was_building_) {
        ImGui::SetNextWindowFocus();
        scroll_to_bottom_ = true;
    }
    was_building_ = building;
    return Panel::begin_window();
}

void BuildOutputPanel::on_imgui() {
    draw_status();
    ImGui::Separator();

    if (host_.build_log().empty()) {
        ImGui::TextDisabled("No build yet.");
        ImGui::TextDisabled("Scripts > Rebuild & Reload, or Build & Reload in "
                            "the Scripts panel.");
        return;
    }

    // Compiler output is column aligned: monospace, like the code editor.
    ImGui::PushFont(ui::load_fonts().mono, 0.f);
    if (ImGui::BeginChild("##build_log",
                          ImVec2{0.f, 0.f},
                          ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.78f, 0.78f, 0.80f, 1.f});
        ImGui::TextUnformatted(host_.build_log().c_str());
        ImGui::PopStyleColor();

        // Follow the tail while a build runs, and once after it finishes, so
        // the error at the end of a failed build is on screen without
        // scrolling.
        const std::size_t length = host_.build_log().size();
        if (length != shown_length_) {
            shown_length_ = length;
            scroll_to_bottom_ = true;
        }
        if (auto_scroll_ && (host_.is_building() || scroll_to_bottom_)) {
            ImGui::SetScrollHereY(1.f);
            if (!host_.is_building()) {
                scroll_to_bottom_ = false;
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();
}

void BuildOutputPanel::draw_status() {
    const bool building = host_.is_building();

    ImGui::BeginDisabled(building || !host_.is_loaded());
    if (ImGui::Button("Rebuild")) {
        host_.begin_build();
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Compile VXScripts.dll on a worker thread and hot reload it.");
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);

    ImGui::SameLine();
    if (building) {
        // Spinner, same shape as the Scripts panel: the build is asynchronous.
        static const char *kFrames[] = {"|", "/", "-", "\\"};
        const int frame = static_cast<int>(ImGui::GetTime() * 10.0) % 4;
        ImGui::TextColored(ImVec4{0.95f, 0.8f, 0.35f, 1.f},
                           "%s building... %.1f s",
                           kFrames[frame],
                           host_.build_seconds());
        return;
    }

    if (host_.build_log().empty()) {
        ImGui::TextDisabled("nothing built yet");
        return;
    }

    char text[128];
    std::snprintf(text,
                  sizeof(text),
                  "last build %s in %.2f s%s",
                  host_.last_build_ok() ? "succeeded" : "FAILED",
                  host_.build_seconds(),
                  host_.last_build_reconfigured() ? " (reconfigured)" : "");
    ImGui::TextColored(host_.last_build_ok() ? ImVec4{0.55f, 0.85f, 0.55f, 1.f}
                                             : ImVec4{1.f, 0.45f, 0.45f, 1.f},
                       "%s",
                       text);
}

} // namespace ivx
