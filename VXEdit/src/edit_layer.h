#pragma once
#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include <imgui.h>
#include <ivx.h>
#include <vector>

namespace ivx {
class EditLayer : public Layer {
  public:
    EditLayer();
    virtual void on_attach() override;
    virtual void on_detach() override;
    virtual void on_update(float delta_time) override;
    virtual void on_event(Event &event) override;
    virtual void on_render() override;

  private:
    void draw_settings_panel();

    Ref<OrthoGraphicCameraController> camera_controller_;
    glm::vec4 color_{1.f};
    Ref<Texture2D> texture_;
    Ref<SubTexture2D> sub_texture_;
    Ref<FrameBuffer> frame_buffer_;

    Ref<Scene> scene_;
    Entity square_entity_;
    Entity camera_entity_;
    Entity second_camera_entity_;

    bool primary_camera_{true};

    bool viewport_focused_{false};
    bool viewport_hovered_{false};
    bool visible_{false};
    ImVec2 viewport_size_{0.f, 0.f};
};
} // namespace ivx