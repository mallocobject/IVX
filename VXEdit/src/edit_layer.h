#pragma once
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include "panel/camera_panel.h"
#include "panel/console_panel.h"
#include "panel/editor_context.h"
#include "panel/inspector_panel.h"
#include "panel/panel.h"
#include "panel/script_panel.h"
#include "panel/scene_hierarchy_panel.h"
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
    virtual void on_render() override;

  private:
    // Panel content. The window itself (Begin/End, padding, visibility) is
    // owned by Panel, these functions only emit widgets.
    void draw_viewport_content();

    void setup_panels();

    // Set when the default layout was (re)built: the viewport tab has to be brought
    // to the front once the panels exist.
    bool pending_viewport_focus_{false};

    // Starts a non-blocking rebuild of VXScripts.dll; poll_scripts() finishes it.
    void reload_scripts();
    // Called every frame: joins a finished build, loads the new module and
    // re-binds the scripts by name so a code change goes live without restarting.
    void poll_scripts();

    Ref<Texture2D> texture_;
    Ref<SubTexture2D> sub_texture_;
    Ref<FrameBuffer> frame_buffer_;

    // Shared editor state (scene + selection). Panels reference it, so it must
    // outlive them: declared before panels_ and never re-assigned afterwards.
    EditorContext context_;

    Ref<Scene> scene_;
    Entity square_entity_;
    Entity camera_entity_;
    Entity second_camera_entity_;

    bool viewport_focused_{false};
    bool viewport_hovered_{false};
    // Set while the viewport panel draws; the input gate is cleared when a frame
    // passes without it (closed/collapsed viewport).
    bool viewport_sampled_{false};
    bool visible_{false};
    ImVec2 viewport_size_{0.f, 0.f};

    bool layout_initialized_{false};
    bool reset_layout_requested_{false};

    PanelManager panels_;
    Ref<SceneHierarchyPanel> scene_hierarchy_panel_;
    Ref<InspectorPanel> inspector_panel_;
    Ref<CameraPanel> camera_panel_;
    Ref<ConsolePanel> console_panel_;
    Ref<StatsPanel> stats_panel_;
    Ref<ScriptPanel> script_panel_;

    ScriptHost scripts_;
};
} // namespace ivx
