#pragma once
#include <ivx.h>
#include <vector>

class Sandbox2D : public ivx::Layer {
  public:
    Sandbox2D();
    virtual void on_attach() override;
    virtual void on_detach() override;
    virtual void on_update(float delta_time) override;
    virtual void on_event(ivx::Event &event) override;
    virtual void on_render() override;

  private:
    void draw_settings_panel();

    ivx::Ref<ivx::OrthoGraphicCameraController> camera_controller_;
    glm::vec4 color_{1.f};
    ivx::Ref<ivx::Texture2D> texture_;
    ivx::Ref<ivx::SubTexture2D> sub_texture_;
};