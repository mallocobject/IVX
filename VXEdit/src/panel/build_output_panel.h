#pragma once

#include "panel.h"

#include <string>
#include <string_view>
#include <vector>

namespace ivx {
class ScriptHost;

// Compiler output of the script module, docked as a sibling tab of the Console.
//
// It lives next to the Console on purpose: both are "text the engine produced",
// and the Console no longer echoes the whole build log (that was duplicated
// output). The Scripts panel keeps the library/editor split and just reports
// build state in its status row.
class BuildOutputPanel : public Panel {
  public:
    explicit BuildOutputPanel(ScriptHost &host);

  protected:
    void on_imgui() override;
    // Brings the tab to the front when a build starts; must run before Begin(),
    // which is why it is not part of on_imgui().
    bool begin_window() override;
    // Status row: rebuild button, auto-scroll, spinner or last result.
    void draw_status();

  private:
    // Puts the selected line - or the whole log when nothing is selected - on
    // the clipboard.
    void copy_selected_or_all();

    ScriptHost &host_;

    // The build log is stable while idle, so the widget only re-reads it when
    // the length changes instead of copying it every frame.
    std::size_t shown_length_{0};
    bool scroll_to_bottom_{false};
    bool auto_scroll_{true};
    // Idle -> building edge, used to bring the tab to the front.
    bool was_building_{false};

    // Line view of the log, rebuilt when its length changes. string_views keep
    // the split cheap: the text itself stays in ScriptHost.
    std::vector<std::string_view> lines_;
    int selected_{-1};
};

} // namespace ivx
