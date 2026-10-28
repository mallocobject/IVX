#include "console_panel.h"

#include <cctype>
#include <cstdio>
#include <imgui.h>

#include "ui/theme.h"
#include <string>
#include <vector>

namespace ivx {
namespace {
const char *level_label(int level) {
    switch (static_cast<LogLevel>(level)) {
    case LogLevel::VERBOSE:
        return "Verbose";
    case LogLevel::DEBUG:
        return "Debug";
    case LogLevel::INFO:
        return "Info";
    case LogLevel::WARN:
        return "Warning";
    case LogLevel::ERROR:
        return "Error";
    case LogLevel::FATAL:
        return "Fatal";
    }
    return "?";
}

ImVec4 level_color(LogLevel level) {
    switch (level) {
    case LogLevel::VERBOSE:
        return ImVec4{0.55f, 0.55f, 0.55f, 1.f};
    case LogLevel::DEBUG:
        return ImVec4{0.55f, 0.68f, 0.85f, 1.f};
    case LogLevel::INFO:
        return ImVec4{0.85f, 0.85f, 0.85f, 1.f};
    case LogLevel::WARN:
        return ImVec4{1.00f, 0.82f, 0.30f, 1.f};
    case LogLevel::ERROR:
        return ImVec4{1.00f, 0.42f, 0.42f, 1.f};
    case LogLevel::FATAL:
        return ImVec4{1.00f, 0.25f, 0.65f, 1.f};
    }
    return ImVec4{1.f, 1.f, 1.f, 1.f};
}

bool contains_case_insensitive(const std::string &haystack,
                               const char *needle) {
    if (needle == nullptr || needle[0] == '\0') {
        return true;
    }

    const std::string lower_needle = [needle] {
        std::string s{needle};
        for (char &c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    }();

    std::string lower_haystack = haystack;
    for (char &c : lower_haystack) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower_haystack.find(lower_needle) != std::string::npos;
}
} // namespace

ConsolePanel::ConsolePanel() : Panel("Console") {
    set_default_size(ImVec2{760.f, 260.f});
}

void ConsolePanel::pump() {
    std::vector<ConsoleMessage> incoming;
    drain_console(incoming);
    if (incoming.empty()) {
        return;
    }

    for (ConsoleMessage &message : incoming) {
        Entry entry;
        entry.level = message.level;
        entry.log_id = message.log_id;
        entry.text = std::move(message.text);
        entries_.push_back(std::move(entry));
        counts_[static_cast<int>(entries_.back().level)]++;
    }

    // Ring buffer: drop the oldest entries, keeping the counts consistent.
    if (entries_.size() > max_entries_) {
        const std::size_t drop = entries_.size() - max_entries_;
        for (std::size_t i = 0; i < drop; i++) {
            counts_[static_cast<int>(entries_[i].level)]--;
        }
        entries_.erase(entries_.begin(),
                       entries_.begin() + static_cast<std::ptrdiff_t>(drop));
    }

    scroll_to_bottom_ = true;
}

void ConsolePanel::on_imgui() {
    draw_toolbar();
    ImGui::Separator();
    draw_entries();
}

void ConsolePanel::draw_toolbar() {
    if (ImGui::Button("Clear")) {
        entries_.clear();
        selected_ = -1;
        for (auto &count : counts_) {
            count = 0;
        }
    }

    ImGui::SameLine();
    // Copies the clicked line, or everything that passes the filters.
    if (ImGui::Button("Copy")) {
        copy_visible_lines();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Copy the selected line to the clipboard.\n"
                          "With nothing selected, copies every visible line.");
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputTextWithHint("##filter", "Filter...", filter_, sizeof(filter_));

    // Level toggles double as counters (Unity-style). Single letters keep all
    // six visible in a narrow panel; the tooltip carries the full name.
    static constexpr const char *kShortLabels[k_level_count] = {
        "V", "D", "I", "W", "E", "F"};
    for (int level = 0; level < k_level_count; level++) {
        ImGui::SameLine();
        ImGui::PushID(level);

        const ImVec4 color = level_color(static_cast<LogLevel>(level));
        if (level_visible_[level]) {
            ImGui::PushStyleColor(ImGuiCol_Text, color);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4{0.45f, 0.45f, 0.45f, 1.f});
        }

        char label[32];
        std::snprintf(label,
                      sizeof(label),
                      "%s %zu",
                      kShortLabels[level],
                      counts_[level]);
        if (ImGui::SmallButton(label)) {
            level_visible_[level] = !level_visible_[level];
        }
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Toggle %s messages", level_label(level));
        }

        ImGui::PopID();
    }
}

void ConsolePanel::draw_entries() {
    // Log lines are columns of similar-looking glyphs: monospace keeps
    // timestamps and banners aligned, at a size a notch smaller than the UI so
    // more of the log fits.
    ImGui::PushFont(ui::load_fonts().mono, ui::log_font_size());

    if (!ImGui::BeginChild("##console_scroll",
                           ImVec2{0.f, 0.f},
                           ImGuiChildFlags_None,
                           ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::EndChild();
        ImGui::PopFont();
        return;
    }

    for (std::size_t i = 0; i < entries_.size(); ++i) {
        const Entry &entry = entries_[i];
        if (!visible(entry.level) || !matches_filter(entry.text)) {
            continue;
        }

        // Selectable instead of TextUnformatted: a clicked line can be copied
        // (Ctrl+C or the Copy button), which is how an error message ends up in
        // a bug report.
        ImGui::PushStyleColor(ImGuiCol_Text, level_color(entry.level));
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(entry.text.c_str(),
                              selected_ == static_cast<int>(i))) {
            selected_ = static_cast<int>(i);
        }
        ImGui::PopID();
        ImGui::PopStyleColor();
    }

    // Ctrl+C while the log has focus: copy the selection, or everything
    // visible.
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C, false)) {
        copy_visible_lines();
    }

    if (scroll_to_bottom_ && auto_scroll_) {
        ImGui::SetScrollHereY(1.0f);
    }
    scroll_to_bottom_ = false;

    ImGui::EndChild();
    ImGui::PopFont();
}

void ConsolePanel::copy_visible_lines() {
    std::string text;
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        const Entry &entry = entries_[i];
        const int index = static_cast<int>(i);
        const bool is_selected = selected_ == index;
        if (!is_selected && (selected_ >= 0 || !visible(entry.level) ||
                             !matches_filter(entry.text))) {
            continue; // a selection narrows the copy down to that one line
        }
        text += entry.text;
        text += '\n';
    }

    if (!text.empty()) {
        ImGui::SetClipboardText(text.c_str());
    }
}

bool ConsolePanel::visible(LogLevel level) const {
    return level_visible_[static_cast<int>(level)];
}

bool ConsolePanel::matches_filter(const std::string &text) const {
    return contains_case_insensitive(text, filter_);
}

} // namespace ivx
