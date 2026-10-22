#pragma once
#include <ivx.h>
#include <vector>

namespace ivx {
class EditorLayer : public Layer {
  public:
    EditorLayer();
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
};
} // namespace ivx