#include "script_panel.h"

#include "ivx/core/application.h"
#include "ivx/core/logger.h"
#include "ivx/core/window.h"
#include "scripts/script_host.h"
#include "scripts/script_registry.h"
#include "ui/theme.h"

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
    ::ShellExecuteW(nullptr,
                    L"open",
                    L"explorer.exe",
                    argument.c_str(),
                    nullptr,
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
        std::string line = log.substr(
            pos, end == std::string::npos ? std::string::npos : end - pos);
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
    load_layout();
}

void ScriptPanel::on_imgui() {
    // The panel keeps drawing while the worker thread compiles, so the finished
    // build is picked up here: refresh the error markers exactly on that edge.
    if (was_building_ && !host_.is_building()) {
        apply_error_markers();
    }
    was_building_ = host_.is_building();

    draw_status();
    ImGui::Separator();

    const float scale = ui::scale();
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x <= 0.f || avail.y <= 0.f) {
        return;
    }

    // Layout: status row above, then [library | splitter | editor] filling the
    // rest. The compiler output lives in its own "Build" tab next to the
    // Console, so this panel is only about picking and editing a script.
    const float splitter = 6.f * scale; // divider thickness
    const float min_body = 90.f * scale;
    const float min_library = 190.f * scale;
    const float min_editor = 240.f * scale;
    const float body_height = std::max(min_body, avail.y);

    // Side by side instead of tabs: while editing you still see the library,
    // and the list is what you pick the next script from. Squeezing the list
    // into a tab (or the editor into a dock column) made one of the two
    // unusable.
    if (library_width_ <= 0.f) {
        library_width_ = std::clamp(
            avail.x * 0.34f, min_library, std::max(min_library, 460.f * scale));
    }
    library_width_ =
        std::clamp(library_width_,
                   min_library,
                   std::max(min_library, avail.x - min_editor - splitter));

    if (ImGui::BeginChild("##library",
                          ImVec2{library_width_, body_height},
                          ImGuiChildFlags_None)) {
        draw_library_pane();
    }
    ImGui::EndChild();

    draw_vertical_splitter(splitter, body_height);

    // A slightly lighter background than the code editor's own: the empty pane
    // has to read as "nothing open yet", not as a broken view.
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          ImVec4{0.169f, 0.169f, 0.176f, 1.f});
    if (ImGui::BeginChild(
            "##editor", ImVec2{0.f, body_height}, ImGuiChildFlags_None)) {
        draw_editor_pane();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Panel-level shortcuts: Ctrl+S writes the file only, Ctrl+B saves and
    // rebuilds the module; keeping them apart matters because the build is the
    // slow half.
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::GetIO().KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            save_script();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_B, false) && !host_.is_building() &&
            !open_script_.empty()) {
            if (save_script()) {
                start_build();
            }
        }
    }
}

void ScriptPanel::draw_vertical_splitter(float width, float height) {
    ImGui::SameLine();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##library_splitter", ImVec2{width, height});
    const bool hot = ImGui::IsItemHovered() || ImGui::IsItemActive();
    if (hot) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }
    if (ImGui::IsItemActive()) {
        library_width_ += ImGui::GetIO().MouseDelta.x;
    }
    if (ImGui::IsItemDeactivated()) {
        save_layout();
    }

    // A 1 px line in the middle of the (invisible) handle, highlighted on hover
    // so the divider is discoverable without adding chrome.
    const float x = pos.x + width * 0.5f;
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2{x, pos.y},
        ImVec2{x, pos.y + height},
        hot ? ImGui::GetColorU32(ImGuiCol_SeparatorHovered)
            : ImGui::GetColorU32(ImGuiCol_Separator),
        1.f);
    ImGui::SameLine();
}

void ScriptPanel::draw_status() {
    if (host_.is_building()) {
        // Spinner: the build is asynchronous, so the panel stays responsive.
        static const char *kFrames[] = {"|", "/", "-", "\\"};
        const int frame = static_cast<int>(ImGui::GetTime() * 10.0) % 4;
        ImGui::TextColored(ImVec4{0.95f, 0.8f, 0.35f, 1.f},
                           "%s Building VXScripts.dll... %.1f s",
                           kFrames[frame],
                           host_.build_seconds());
        ImGui::SameLine();
        ImGui::TextDisabled("%s",
                            host_.last_build_reconfigured()
                                ? "(reconfigured: file set changed)"
                                : "(incremental)");
        return;
    }

    if (!status_.empty()) {
        if (status_is_error_) {
            ImGui::TextColored(
                ImVec4{1.f, 0.45f, 0.45f, 1.f}, "%s", status_.c_str());
        } else {
            ImGui::TextColored(
                ImVec4{0.55f, 0.85f, 0.55f, 1.f}, "%s", status_.c_str());
        }
        return;
    }

    if (!host_.build_log().empty()) {
        char text[128] = {};
        std::snprintf(text,
                      sizeof(text),
                      "Last build %s in %.2f s",
                      host_.last_build_ok() ? "succeeded" : "FAILED",
                      host_.build_seconds());
        if (host_.last_build_ok()) {
            ImGui::TextColored(ImVec4{0.55f, 0.85f, 0.55f, 1.f}, "%s", text);
        } else {
            ImGui::TextColored(ImVec4{1.f, 0.45f, 0.45f, 1.f}, "%s", text);
        }
    }
}

void ScriptPanel::draw_library_pane() {
    draw_toolbar();
    ImGui::Separator();
    draw_new_script_form();
    ImGui::Separator();
    draw_script_list();
}

void ScriptPanel::draw_toolbar() {
    const bool building = host_.is_building();

    ImGui::BeginDisabled(building);
    if (ImGui::Button("Build & Reload")) {
        host_.begin_build();
        status_.clear();
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Compiles only VXScripts.dll on a worker thread and\n"
                          "hot reloads it. The editor stays responsive.");
    }

    ImGui::SameLine();
    if (ImGui::Button("Folder")) {
        reveal_in_explorer(host_.scripts_dir());
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show %s in Explorer",
                          host_.scripts_dir().string().c_str());
    }

    // On its own line: the left pane is narrow, and the module state is
    // secondary information that can be truncated without losing anything
    // useful.
    ImGui::TextDisabled("module: %s", host_.is_loaded() ? "loaded" : "missing");
}

void ScriptPanel::draw_new_script_form() {
    ImGui::TextDisabled("New script");
    // Requested by the empty editor state's "New script" button.
    if (focus_new_script_) {
        ImGui::SetKeyboardFocusHere();
        focus_new_script_ = false;
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint(
        "##new_script", "MyScript", new_script_name_, sizeof(new_script_name_));
    ImGui::BeginDisabled(host_.is_building());
    if (ImGui::Button("Create + Build", ImVec2{-FLT_MIN, 0.f})) {
        std::filesystem::path header_path;
        std::filesystem::path source_path;
        std::string error;

        const std::string name = new_script_name_;
        if (!host_.create_script(name, header_path, source_path, error)) {
            status_ = error;
            status_is_error_ = true;
        } else {
            host_.begin_build();
            status_.clear();
            new_script_name_[0] = '\0';
            open_script(name);
        }
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Writes scripts/<Name>.h/.cpp from the template and\n"
                          "compiles the module.");
    }
}

void ScriptPanel::draw_script_list() {
    const auto &entries = ScriptRegistry::get().entries();
    ImGui::Text("Library (%zu)", entries.size());

    if (entries.empty()) {
        ImGui::TextDisabled("No scripts registered. Is VXScripts.dll built?");
        return;
    }

    // Same split as the pane itself: the list grows, the build output keeps a
    // fixed slot at the bottom.
    constexpr float kBuildOutputHeight = 150.f;
    if (!ImGui::BeginChild("##script_list",
                           ImVec2{0.f, -kBuildOutputHeight},
                           ImGuiChildFlags_None)) {
        ImGui::EndChild();
        return;
    }

    // Two columns instead of a full-width Selectable with SameLine buttons: the
    // Selectable used to cover the whole row and swallow every click meant for
    // the buttons. A single click opens the script, the way a file gets opened
    // in an editor's explorer.
    const ImGuiTableFlags flags =
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings;
    if (ImGui::BeginTable("##scripts", 2, flags)) {
        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(
            "actions", ImGuiTableColumnFlags_WidthFixed, 64.f);

        for (const ScriptEntry &entry : entries) {
            ImGui::PushID(entry.name.c_str());
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable(entry.name.c_str(),
                                  open_script_ == entry.name,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                open_script(entry.name);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(
                    "%s", host_.script_path(entry.name).string().c_str());
            }

            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Folder")) {
                reveal_in_explorer(host_.script_path(entry.name));
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();
}

void ScriptPanel::draw_empty_editor() {
    // Centred "what to do next" block instead of an empty black column: the
    // pane is useless without a file, so it should explain itself and offer the
    // two things a user can reasonably do from here.
    const ImVec2 available = ImGui::GetContentRegionAvail();
    ImGuiStyle &style = ImGui::GetStyle();

    const float glyph_size = style.FontSizeBase * 2.6f;
    const float line_height = ImGui::GetTextLineHeightWithSpacing();
    const float button_height = ImGui::GetFrameHeight();

    const char *title = "No script open";
    const char *hint =
        "Pick one in the library on the left, or create a new one";
    const char *shortcuts = "Ctrl+S  save        Ctrl+B  save & build";

    auto centered_text = [&available](const char *text, ImVec4 color) {
        const float width = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                             std::max(0.f, (available.x - width) * 0.5f));
        ImGui::TextColored(color, "%s", text);
    };

    auto button_width = [&style](const char *label) {
        return ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.f;
    };
    const float new_width = button_width("New script");
    const float build_width = button_width("Build & Reload");
    const float row_width = new_width + style.ItemSpacing.x + build_width;

    // Total height, used to centre the block vertically.
    const float block_height = glyph_size + line_height * 2.f + button_height +
                               ImGui::GetTextLineHeight() +
                               style.ItemSpacing.y * 4.f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                         std::max(0.f, (available.y - block_height) * 0.5f));

    // Big dim braces: reads as "code goes here" without an icon font.
    ImGui::PushFont(nullptr, glyph_size);
    const float glyph_width = ImGui::CalcTextSize("{ }").x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         std::max(0.f, (available.x - glyph_width) * 0.5f));
    ImGui::TextColored(ImVec4{0.34f, 0.36f, 0.42f, 1.f}, "{ }");
    ImGui::PopFont();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
    centered_text(title, ImGui::GetStyleColorVec4(ImGuiCol_Text));
    centered_text(hint, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         std::max(0.f, (available.x - row_width) * 0.5f));
    if (ImGui::Button("New script", ImVec2{new_width, 0.f})) {
        // The name field lives in the left pane, which is drawn before this
        // one, so the request is picked up on the next frame.
        focus_new_script_ = true;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(host_.is_building());
    if (ImGui::Button("Build & Reload", ImVec2{build_width, 0.f})) {
        start_build();
    }
    ImGui::EndDisabled();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
    centered_text(shortcuts, ImVec4{0.45f, 0.45f, 0.48f, 1.f});
}

void ScriptPanel::draw_editor_pane() {
    if (!editor_ready_ || open_script_.empty()) {
        draw_empty_editor();
        return;
    }

    const bool dirty = is_dirty();
    const bool building = host_.is_building();

    if (ImGui::Button("Save")) {
        save_script();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Writes the file only (Ctrl+S).\n"
                          "Use Build & Reload to compile it.");
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(building);
    if (ImGui::Button("Save & Build")) {
        if (save_script()) {
            start_build();
        }
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Saves, then compiles VXScripts.dll on a worker thread\n"
            "and hot reloads it (Ctrl+B).");
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

    // Code is column-aligned by nature: use the monospace face even though the
    // rest of the editor is proportional.
    ImGui::PushFont(ui::load_fonts().mono, 0.f);
    editor_.Render("##code", ImVec2{-FLT_MIN, -FLT_MIN}, false);
    ImGui::PopFont();

    // The editor is a custom widget, not an ImGui InputText: ImGui only asks
    // the SDL backend to start text input for its own InputText widgets, so
    // without this no SDL_EVENT_TEXT_INPUT ever arrives and typing does
    // nothing. The widget sets io.WantTextInput while it owns the keyboard.
    Application::get().get_window().set_text_input_enabled(
        ImGui::GetIO().WantTextInput);
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
    // Take the baseline from the widget, not from the file: the editor splits
    // the text into lines and joins them with '\n', so a CRLF file would
    // otherwise look modified right after opening.
    saved_text_ = editor_.GetText();
    open_script_ = name;
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

    // Skipping the write when nothing changed is not just a micro-optimisation:
    // rewriting the file bumps its timestamp, which makes the build system
    // recompile a script that did not change.
    if (!is_dirty()) {
        status_ = path.filename().string() + " is unchanged";
        status_is_error_ = false;
        return true;
    }

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

void ScriptPanel::load_layout() {
    // Only the library column width is persisted now: the compiler output moved
    // to its own "Build" panel next to the Console.
    std::ifstream file{SCRIPTS_PANEL_LAYOUT};
    float width = 0.f;
    if (file >> width) {
        library_width_ = width;
    }
}

void ScriptPanel::save_layout() const {
    // Next to layout.ini so it survives being launched from anywhere.
    std::ofstream file{SCRIPTS_PANEL_LAYOUT, std::ios::trunc};
    if (file) {
        file << library_width_ << "\n";
    }
}

void ScriptPanel::start_build() {
    if (!host_.begin_build()) {
        return; // already building
    }
    status_.clear();
    // Old error markers are stale the moment a new build starts.
    if (editor_ready_) {
        editor_.SetErrorMarkers({});
    }
}

void ScriptPanel::apply_error_markers() {
    if (!editor_ready_ || open_script_.empty() || host_.build_log().empty()) {
        return;
    }
    editor_.SetErrorMarkers(
        parse_error_markers(host_.build_log(), open_script_ + ".cpp"));
}

} // namespace ivx
