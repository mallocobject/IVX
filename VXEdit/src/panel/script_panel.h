#pragma once

#include "panel.h"

#include <string>
#include <vector>

#include <TextEditor.h>

namespace ivx {
class ScriptHost;

// Script panel: the module's script library, a "New Script" form that generates
// C++ sources from a template, an embedded code editor with syntax highlighting,
// and a build & reload button (hot reload) with the compiler output.
//
// Docked next to the Viewport, Godot style: the file being edited shows up as a
// sibling tab of the viewport.
class ScriptPanel : public Panel {
  public:
    explicit ScriptPanel(ScriptHost &host);

  protected:
    void on_imgui() override;

  private:
    void draw_status();
    void draw_library_tab();
    void draw_editor_tab();
    void draw_toolbar();
    void draw_script_list();
    void draw_new_script_form();
    void draw_build_output();

    void open_script(const std::string &name);
    bool is_dirty() const;
    bool save_script();
    // Hands the build to the host's worker thread; the panel keeps drawing.
    void start_build();
    // Marks the lines the compiler complained about in the open file.
    void apply_error_markers();

    ScriptHost &host_;

    TextEditor editor_;
    bool editor_ready_{false};
    // Set when a script is opened: the editor tab must become the active one,
    // otherwise the user keeps looking at the Library tab.
    bool focus_editor_tab_{false};
    // Detects the building -> idle edge so the error markers refresh exactly once.
    bool was_building_{false};
    // Asks for this panel's dock tab to come to the front on the next frame (the
    // dock tab is created by the frame that opens the script).
    bool request_dock_focus_{false};

    char new_script_name_[64]{};
    std::string status_;
    bool status_is_error_{false};
    bool show_build_log_{false};
    std::string open_script_;
    // Content as of the last load/save, used for the dirty marker: the widget's
    // own IsTextChanged() flag is not reset by SetText in this fork.
    std::string saved_text_;
};

} // namespace ivx
