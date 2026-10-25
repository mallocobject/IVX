#pragma once

#include "panel.h"

#include <string>
#include <vector>

#include <TextEditor.h>

namespace ivx {
class ScriptHost;

// Script panel: the module's script library, a "New Script" form that generates
// C++ sources from a template, an embedded code editor with syntax
// highlighting, and a build & reload button (hot reload) with the compiler
// output.
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
    void draw_library_pane();
    void draw_editor_pane();
    // Placeholder for the editor column while no script is open.
    void draw_empty_editor();
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
    // Detects the building -> idle edge so the error markers refresh exactly
    // once.
    bool was_building_{false};
    // Set by the empty state's "New script" button; the name field picks it up
    // on the next frame (the library column is drawn before the editor column).
    bool focus_new_script_{false};

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
