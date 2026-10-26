#pragma once
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include "panel/console_panel.h"
#include "panel/editor_context.h"
#include "panel/inspector_panel.h"
#include "panel/panel.h"
#include "panel/scene_hierarchy_panel.h"
#include "panel/script_panel.h"
#include "panel/stats_panel.h"
#include "scripts/script_host.h"
#include <imgui.h>
#include <ivx.h>
#include <vector>

namespace ivx {
class EditLayer : public Layer {
  public:
    EditLayer();
    virtual void on_attach() override;
    virtual void on_detach() override;
    virtual void on_update(float dt) override;
    virtual void on_event(Event &event) override;
    // UI phase: menu bar, status bar, dock space and every panel. The scene
    // itself is rendered into the viewport framebuffer from on_update().
    virtual void on_imgui() override;

  private:
    // Panel content. The window itself (Begin/End, padding, visibility) is
    // owned by Panel, these functions only emit widgets.
    void draw_viewport_content();

    void setup_panels();

    // Main menu bar (own viewport row) and the bottom status bar.
    void draw_menu_bar();
    void draw_status_bar();

    // Starts a non-blocking rebuild of VXScripts.dll; poll_scripts() finishes
    // it.
    void reload_scripts();
    // Called every frame: joins a finished build, loads the new module and
    // re-binds the scripts by name so a code change goes live without
    // restarting.
    void poll_scripts();

    // Re-evaluates whether the loaded module is older than its sources. Only a
    // hint: rebuilding stays the user's call (Ctrl+B). Called at startup, after
    // every build and periodically so an edit made while the editor runs shows
    // up too.
    void refresh_scripts_staleness();

    // Binds CameraController to the primary camera once per session (see the
    // comment in the implementation).
    void bind_default_camera_script();

    // Ref<Texture2D> texture_;
    // Ref<SubTexture2D> sub_texture_;
    Ref<FrameBuffer> frame_buffer_;

    // Shared editor state (scene + selection). Panels reference it, so it must
    // outlive them: declared before panels_ and never re-assigned afterwards.
    EditorContext context_;

    Ref<Scene> scene_;

    // Set when the default layout was (re)built: the viewport tab has to be
    // brought to the front once the panels exist. Starts true: the viewport is
    // the tab you want on startup, no matter which tab the saved layout.ini
    // restored.
    bool pending_viewport_focus_{true};

    bool viewport_focused_{false};
    // True while the viewport owns mouse/keyboard input; latched for
    // on_event().
    bool input_owned_{false};
    bool viewport_hovered_{false};
    // Set while the viewport panel draws; the input gate is cleared when a
    // frame passes without it (closed/collapsed viewport).
    bool viewport_sampled_{false};
    bool visible_{false};
    ImVec2 viewport_size_{0.f, 0.f};

    bool layout_initialized_{false};
    bool reset_layout_requested_{false};
    // Set once the default camera controller has been bound (or found already
    // bound), so unbinding it in the inspector sticks.
    bool default_script_bound_{false};
    // True while some script source is newer than the loaded module (shows a
    // hint in the status bar; never triggers a build on its own).
    bool scripts_stale_{false};
    // Frame counter for the periodic staleness re-check (~2 s at 60 fps).
    int staleness_tick_{0};

    PanelManager panels_;
    Ref<SceneHierarchyPanel> scene_hierarchy_panel_;
    Ref<InspectorPanel> inspector_panel_;
    Ref<ConsolePanel> console_panel_;
    Ref<StatsPanel> stats_panel_;
    Ref<ScriptPanel> script_panel_;
    // The viewport is a LambdaPanel; kept so the stats overlay can tell whether
    // the viewport tab is the one currently on screen.
    Ref<Panel> viewport_panel_;

    ScriptHost scripts_;
};
} // namespace ivx
