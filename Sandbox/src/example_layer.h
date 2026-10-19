#pragma once

#include <ivx.h>

class ExampleLayer : public ivx::Layer {
  public:
    ExampleLayer();
    void on_update(float delta_time) override;
    void on_render() override;
    void on_event(ivx::Event &event) override;

  private:
    ivx::Ref<ivx::Shader> shader_;

    ivx::Ref<ivx::VertexArray> vertex_array_;

    ivx::Ref<ivx::OrthoGraphicCameraController> camera_controller_;

    ivx::Ref<ivx::Texture2D> texture_;
    ivx::Ref<ivx::Texture2D> texture2_;

    glm::vec3 color_{1.f};
};
