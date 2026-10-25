#pragma once

#include "editor_context.h"
#include "ivx/scene/entity.h"
#include "panel.h"

namespace ivx {

// Camera panel, laid out like the camera sections of Unity/Unreal/Hazel:
//   1. the list of scene cameras (radio = primary, exactly one),
//   2. the selected camera's projection parameters (shared with the Inspector's
//      Camera component UI so both stay in sync),
//   3. view helpers: where the camera is, framing the selection,
//   4. the raw projection matrix, for when "nothing renders".
class CameraPanel : public Panel {
  public:
    explicit CameraPanel(EditorContext &context);

  protected:
    void on_imgui() override;

  private:
    void draw_camera_list();
    void draw_camera_details(Entity entity);
    void draw_view_helpers(Entity entity);

    Entity primary_camera() const;
    void set_primary_camera(Entity entity);
    // Cameras are entities: "Add Component" deliberately does not offer the
    // Camera component, this button is the supported way to make one.
    void create_camera();
    // Moves the camera onto the selection (and frames it when orthographic).
    void focus_selection(Entity camera_entity, bool also_frame);

    EditorContext &context_;
    entt::entity selected_camera_{entt::null};
};

} // namespace ivx
