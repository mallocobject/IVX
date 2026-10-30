#pragma once
#include "editor_camera.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include "panel/console_panel.h"
#include "panel/editor_context.h"
#include "panel/inspector_panel.h"
#include "panel/panel.h"
#include "panel/scene_hierarchy_panel.h"
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

    // ---- transform gizmo (ImGuizmo) ----
    enum class GizmoMode { Translate = 1, Rotate, Scale };

    // Draws the gizmo for the selected entity on top of the viewport image.
    // `image_min` and `image_size` come from the ImGui::Image() call, so the
    // handles line up with what the camera rendered.
    void draw_gizmo(const ImVec2 &image_min, const ImVec2 &image_size);
    // Writes the drag back into the entity's local transform: translation and
    // scale are read from `world_matrix`, rotation is composed from this
    // frame's increment (see the comment in the implementation for why
    // ImGuizmo's deltaMatrix cannot be used here).
    static void apply_gizmo_matrix(Entity entity,
                                   const glm::mat4 &world_matrix);
    int gizmo_operation() const;

    void setup_panels();

    // Main menu bar (own viewport row) and the bottom status bar.
    void draw_menu_bar();
    void draw_status_bar();

    void create_scene();

    // File > Open / Save Scene. Both go through ivx::Serializer and use a
    // fixed path under the editor assets: there is no file dialog yet.
    void load_scene(const std::filesystem::path &filepath);
    void save_scene(const std::filesystem::path &filepath);

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

    // Which handles the gizmo shows. Keys 1/2/3 switch it while the viewport
    // owns the input; the menu shows the same three entries.
    GizmoMode gizmo_mode_{GizmoMode::Translate};

    bool layout_initialized_{false};
    bool reset_layout_requested_{false};

    PanelManager panels_;
    Ref<SceneHierarchyPanel> scene_hierarchy_panel_;
    Ref<InspectorPanel> inspector_panel_;
    Ref<ConsolePanel> console_panel_;
    // The viewport is a LambdaPanel; kept so the input gate can tell whether
    // the viewport tab is the one currently on screen.
    Ref<Panel> viewport_panel_;

    // The viewport's pass: target, clear values and pipeline state in one
    // object. Declared before editor_camera_ so it is destroyed after it (the
    // pass does not reference the camera, but keeping the render state objects
    // together reads better).
    Ref<RenderPass> render_pass_;

    EditorCamera editor_camera_;
};
} // namespace ivx
