#include "script_panel.h"

#include "ivx/core/application.h"
#include "ivx/core/logger.h"
#include "ivx/core/window.h"
#include "scripts/script_host.h"
#include "scripts/script_registry.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iterator>
#include <string>

#if defined(IVX_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace ivx {
namespace {

void reveal_in_explorer(const std::filesystem::path &path) {
#if defined(IVX_PLATFORM_WINDOWS)
    const std::wstring argument = L"/select,\"" + path.wstring() + L"\"";
    ::ShellExecuteW(nullptr, L"open", L"explorer.exe", argument.c_str(), nullptr,
                    SW_SHOWNORMAL);
#else
    (void)path;
#endif
}

bool read_file(const std::filesystem::path &path, std::string &out) {
    std::ifstream file{path, std::ios::binary};
    if (!file) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>{file},
               std::istreambuf_iterator<char>{});
    return true;
}

bool write_file(const std::filesystem::path &path, const std::string &content) {
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file) {
        return false;
    }
    file << content;
    return file.good();
}

// Pulls "file(line,col): error ..." pairs out of the MSBuild output.
TextEditor::ErrorMarkers parse_error_markers(const std::string &log,
                                             const std::string &file_stem) {
    TextEditor::ErrorMarkers markers;

    std::size_t pos = 0;
    while (pos < log.size()) {
        const std::size_t end = log.find('\n', pos);
        std::string line = log.substr(pos, end == std::string::npos
                                               ? std::string::npos
                                               : end - pos);
        pos = end == std::string::npos ? log.size() : end + 1;

        if (line.find(": error ") == std::string::npos &&
            line.find(": warning ") == std::string::npos) {
            continue;
        }
        if (line.find(file_stem) == std::string::npos) {
            continue;
        }

        const std::size_t open = line.rfind('(', line.find("): "));
        if (open == std::string::npos) {
            continue;
        }
        const std::size_t close = line.find(',', open);
        if (close == std::string::npos) {
            continue;
        }
        const int line_number =
            std::atoi(line.substr(open + 1, close - open - 1).c_str());
        if (line_number <= 0) {
            continue;
        }
        markers[line_number] = line.substr(line.find("): ") + 3);
    }
    return markers;
}

} // namespace

ScriptPanel::ScriptPanel(ScriptHost &host) : Panel("Scripts"), host_(host) {
    set_default_size(ImVec2{700.f, 460.f});
}

void ScriptPanel::on_imgui() {
    if (!status_.empty()) {
        if (status_is_error_) {
            ImGui::TextColored(ImVec4{1.f, 0.45f, 0.45f, 1.f}, "%s",
                               status_.c_str());
        } else {
            ImGui::TextColored(ImVec4{0.55f, 0.85f, 0.55f, 1.f}, "%s",
                               status_.c_str());
        }
    }

    if (!ImGui::BeginTabBar("##script_tabs")) {
        return;
    }

    if (ImGui::BeginTabItem("Library")) {
        draw_library_tab();
        ImGui::EndTabItem();
    }

    if (!open_script_.empty()) {
        const bool dirty = is_dirty();
        const std::string title = open_script_ + ".cpp" + (dirty ? " *" : "") +
                                  "###script_editor";
        ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
        if (focus_editor_tab_) {
            flags |= ImGuiTabItemFlags_SetSelected;
            focus_editor_tab_ = false;
        }
        if (ImGui::BeginTabItem(title.c_str(), nullptr, flags)) {
            draw_editor_tab();
            ImGui::EndTabItem();
        }
    }

    ImGui::EndTabBar();
}

void ScriptPanel::draw_library_tab() {
    draw_toolbar();
    ImGui::Separator();
    draw_new_script_form();
    ImGui::Separator();
    draw_script_list();
    ImGui::Separator();
    draw_build_output();
}

void ScriptPanel::draw_toolbar() {
    if (ImGui::Button("Build & Reload")) {
        build_and_reload();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Rebuilds VXScripts.dll and re-binds every script.\n"
                          "Use it after editing sources outside the editor.");
    }

    ImGui::SameLine();
    if (ImGui::Button("Open Folder")) {
        reveal_in_explorer(host_.scripts_dir());
    }

    ImGui::SameLine();
    ImGui::TextDisabled("module: %s | %s",
                        host_.is_loaded() ? "loaded" : "not loaded",
                        host_.scripts_dir().filename().string().c_str());
}

void ScriptPanel::draw_new_script_form() {
    ImGui::TextDisabled("New script (generates scripts/<Name>.h/.cpp)");

    ImGui::SetNextItemWidth(220.f);
    ImGui::InputTextWithHint("##new_script", "MyScript", new_script_name_,
                             sizeof(new_script_name_));
    ImGui::SameLine();
    if (ImGui::Button("Create + Build")) {
        std::filesystem::path header_path;
        std::filesystem::path source_path;
        std::string error;

        const std::string name = new_script_name_;
        if (!host_.create_script(name, header_path, source_path, error)) {
            status_ = error;
            status_is_error_ = true;
        } else {
            build_and_reload();
            status_ = "Created " + source_path.filename().string() +
                      (last_build_ok_ ? " - build OK" : " - build FAILED");
            status_is_error_ = !last_build_ok_;
            new_script_name_[0] = '\0';
            open_script(name);
        }
    }
}

void ScriptPanel::draw_script_list() {
    const auto &entries = ScriptRegistry::get().entries();
    ImGui::Text("Library (%zu)", entries.size());

    if (entries.empty()) {
        ImGui::TextDisabled("No scripts registered. Is VXScripts.dll built?");
        return;
    }

    if (!ImGui::BeginChild("##script_list", ImVec2{0.f, 120.f},
                           ImGuiChildFlags_None)) {
        ImGui::EndChild();
        return;
    }

    for (const ScriptEntry &entry : entries) {
        ImGui::PushID(entry.name.c_str());

        ImGui::Selectable(entry.name.c_str(), open_script_ == entry.name,
                          ImGuiSelectableFlags_AllowDoubleClick);
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            open_script(entry.name);
        }

        ImGui::SameLine(ImGui::GetContentRegionMax().x - 108.f);
        if (ImGui::SmallButton("Open")) {
            open_script(entry.name);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Folder")) {
            reveal_in_explorer(host_.script_path(entry.name));
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void ScriptPanel::draw_editor_tab() {
    if (!editor_ready_) {
        ImGui::TextDisabled("Open a script from the Library tab.");
        return;
    }

    const bool dirty = is_dirty();

    if (ImGui::Button("Save")) {
        save_script();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Writes the file only (Ctrl+S).\n"
                          "Use Build & Reload to compile it.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Save & Build")) {
        if (save_script()) {
            build_and_reload();
            apply_error_markers();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Saves, then compiles VXScripts.dll and hot reloads it\n"
                          "(Ctrl+B). Only the script module is rebuilt.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert") && !open_script_.empty()) {
        open_script(open_script_); // reload from disk
    }
    ImGui::SameLine();
    if (dirty) {
        ImGui::TextColored(ImVec4{1.f, 0.8f, 0.3f, 1.f}, "unsaved");
    } else {
        ImGui::TextDisabled("saved");
    }

    ImGui::Separator();

    editor_.Render("##code", ImVec2{-FLT_MIN, -FLT_MIN}, false);

    // The editor is a custom widget, not an ImGui InputText: ImGui only asks the
    // SDL backend to start text input for its own InputText widgets, so without
    // this no SDL_EVENT_TEXT_INPUT ever arrives and typing does nothing. The
    // widget sets io.WantTextInput while it owns the keyboard.
    Application::get().get_window().set_text_input_enabled(
        ImGui::GetIO().WantTextInput);

    // Shortcuts: Ctrl+S writes the file only, Ctrl+B saves and rebuilds the
    // module. Keeping them apart matters because the build is the slow half.
    const bool editor_window_focused =
        ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    if (editor_window_focused && ImGui::GetIO().KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            save_script();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_B, false)) {
            if (save_script()) {
                build_and_reload();
                apply_error_markers();
            }
        }
    }
}

void ScriptPanel::open_script(const std::string &name) {
    std::string text;
    const std::filesystem::path path = host_.script_path(name);
    if (!read_file(path, text)) {
        status_ = "cannot read " + path.string();
        status_is_error_ = true;
        return;
    }

    if (!editor_ready_) {
        editor_.SetLanguageDefinition(
            TextEditor::LanguageDefinition::CPlusPlus());

        // Tune the palette to the editor's dark theme.
        TextEditor::Palette palette = editor_.GetPalette();
        palette[(int)TextEditor::PaletteIndex::Background] =
            IM_COL32(24, 24, 28, 255);
        palette[(int)TextEditor::PaletteIndex::Default] =
            IM_COL32(220, 220, 220, 255);
        palette[(int)TextEditor::PaletteIndex::Comment] =
            IM_COL32(106, 153, 85, 255);
        palette[(int)TextEditor::PaletteIndex::Keyword] =
            IM_COL32(86, 156, 214, 255);
        palette[(int)TextEditor::PaletteIndex::String] =
            IM_COL32(206, 145, 120, 255);
        palette[(int)TextEditor::PaletteIndex::Preprocessor] =
            IM_COL32(155, 155, 155, 255);
        palette[(int)TextEditor::PaletteIndex::Identifier] =
            IM_COL32(220, 220, 220, 255);
        palette[(int)TextEditor::PaletteIndex::Number] =
            IM_COL32(181, 206, 168, 255);
        palette[(int)TextEditor::PaletteIndex::CurrentLineFill] =
            IM_COL32(40, 40, 46, 255);
        palette[(int)TextEditor::PaletteIndex::Selection] =
            IM_COL32(38, 79, 120, 255);
        editor_.SetPalette(palette);
        editor_ready_ = true;
    }

    editor_.SetText(text);
    editor_.SetCursorPosition({0, 0});
    // Take the baseline from the widget, not from the file: the editor splits the
    // text into lines and joins them with '\n', so a CRLF file would otherwise
    // look modified right after opening.
    saved_text_ = editor_.GetText();
    open_script_ = name;
    focus_editor_tab_ = true;
    status_.clear();
    status_is_error_ = false;
}

bool ScriptPanel::is_dirty() const {
    if (!editor_ready_) {
        return false;
    }
    return editor_.GetText() != saved_text_;
}

bool ScriptPanel::save_script() {
    if (open_script_.empty()) {
        return false;
    }
    const std::filesystem::path path = host_.script_path(open_script_);
    const std::string content = editor_.GetText();
    if (!write_file(path, content)) {
        status_ = "could not write " + path.string();
        status_is_error_ = true;
        return false;
    }
    saved_text_ = content;
    status_ = "Saved " + path.filename().string();
    status_is_error_ = false;
    return true;
}

void ScriptPanel::draw_build_output() {
    if (ImGui::CollapsingHeader("Build output",
                               show_build_log_ ? ImGuiTreeNodeFlags_DefaultOpen
                                               : ImGuiTreeNodeFlags_None)) {
        show_build_log_ = true;
        if (build_log_.empty()) {
            ImGui::TextDisabled("(nothing yet)");
        } else if (ImGui::BeginChild("##build_log", ImVec2{0.f, 0.f})) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.75f, 0.75f, 0.75f, 1.f});
            ImGui::TextUnformatted(build_log_.c_str());
            ImGui::PopStyleColor();
            ImGui::EndChild();
        } else {
            ImGui::EndChild();
        }
    }
}

bool ScriptPanel::build_and_reload() {
    build_log_.clear();
    show_build_log_ = true;

    const bool ok = host_.build_and_reload(build_log_);
    last_build_ok_ = ok;

    char timing[96] = {};
    std::snprintf(timing,
                  sizeof(timing),
                  " in %.2f s%s",
                  host_.last_build_ms(),
                  host_.last_build_reconfigured() ? " (reconfigured)" : "");
    if (ok) {
        status_ = std::string{"Build OK, scripts reloaded"} + timing;
        status_is_error_ = false;
    } else {
        status_ = std::string{"Build FAILED"} + timing + " - see output below";
        status_is_error_ = true;
    }
    apply_error_markers();
    return ok;
}

void ScriptPanel::apply_error_markers() {
    if (!editor_ready_ || open_script_.empty() || build_log_.empty()) {
        return;
    }
    editor_.SetErrorMarkers(
        parse_error_markers(build_log_, open_script_ + ".cpp"));
}

} // namespace ivx
