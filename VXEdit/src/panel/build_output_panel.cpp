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

    // Compiler output is column aligned: monospace, like the code editor, and
    // smaller than the UI so a whole compiler log fits (see ui::kLogFontScale).
    ImGui::PushFont(ui::load_fonts().mono, ui::log_font_size());
    if (ImGui::BeginChild("##build_log",
                          ImVec2{0.f, 0.f},
                          ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        // Re-split only when the log changed; the views point into ScriptHost's
        // string.
        const std::string &log = host_.build_log();
        const std::size_t length = log.size();
        if (length != shown_length_) {
            shown_length_ = length;
            scroll_to_bottom_ = true;
            selected_ = -1;
            lines_.clear();
            std::size_t begin = 0;
            while (begin <= log.size()) {
                const std::size_t end = log.find('\n', begin);
                lines_.emplace_back(
                    log.data() + begin,
                    (end == std::string::npos ? log.size() : end) - begin);
                if (end == std::string::npos) {
                    break;
                }
                begin = end + 1;
            }
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.78f, 0.78f, 0.80f, 1.f});
        for (std::size_t i = 0; i < lines_.size(); ++i) {
            if (lines_[i].empty()) {
                ImGui::Spacing();
                continue;
            }
            ImGui::PushID(static_cast<int>(i));
            // Selectable, not TextUnformatted: a clicked line can be copied,
            // which is what a build log is usually needed for.
            if (ImGui::Selectable(std::string{lines_[i]}.c_str(),
                                  selected_ == static_cast<int>(i))) {
                selected_ = static_cast<int>(i);
            }
            ImGui::PopID();
        }
        ImGui::PopStyleColor();

        // Ctrl+C while the log has focus: the selection, or the whole log.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
            ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C, false)) {
            copy_selected_or_all();
        }

        // Follow the tail while a build runs, and once after it finishes, so
        // the error at the end of a failed build is on screen without
        // scrolling.
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

void BuildOutputPanel::copy_selected_or_all() {
    const std::string &log = host_.build_log();
    if (selected_ >= 0 && static_cast<std::size_t>(selected_) < lines_.size()) {
        const std::string line{lines_[static_cast<std::size_t>(selected_)]};
        ImGui::SetClipboardText(line.c_str());
        return;
    }
    if (!log.empty()) {
        ImGui::SetClipboardText(log.c_str());
    }
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
    // Copies the clicked line, or the whole log.
    ImGui::BeginDisabled(host_.build_log().empty());
    if (ImGui::Button("Copy")) {
        copy_selected_or_all();
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Copy the selected line to the clipboard.\n"
                          "With nothing selected, copies the whole log.");
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
