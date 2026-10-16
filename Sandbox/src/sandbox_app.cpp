#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "sandbox2d.h"
#include <imgui.h>
#include <iostream>
#include <ivx.h>
#include <ivx/core/entry_point.h>

class ExampleLayer : public ivx::Layer {
  public:
    ExampleLayer() : Layer("Example") {
        vertex_array_ = ivx::VertexArray::create();

        float vertices[5 * 4] = {
            -300.f, -300.f, 0.f, 0.f, 0.f, 300.f,  -300.f, 0.f, 1.f, 0.f,
            300.f,  300.f,  0.f, 1.f, 1.f, -300.f, 300.f,  0.f, 0.f, 1.f,
        };

        auto vertex_buf = ivx::VertexBuffer::create(
            vertices, sizeof(vertices) / sizeof(float));

        ivx::Ref<ivx::BufferLayout> bl{new ivx::BufferLayout{
            {"a_Position", ivx::ShaderDataType::vec3, false},
            {"a_Texcoord", ivx::ShaderDataType::vec2, false},
        }};
        vertex_buf->set_buffer_layout(bl);

        vertex_array_->add_vertex_buffer(vertex_buf);

        uint32_t indices[] = {
            0,
            1,
            2,
            0,
            2,
            3,
        };

        auto index_buf = ivx::IndexBuffer::create(
            indices, sizeof(indices) / sizeof(uint32_t));

        vertex_array_->set_index_buffer(index_buf);

        shader_ = ivx::ShaderLibrary::load(SHADERS "texture.glsl");

        ivx::RenderCommand::set_clear_color(glm::vec4(0.f));

        camera_controller_ = ivx::make_ref<ivx::OrthoGraphicCameraController>(
            1080, 720, 1.f, true);

        texture_ = ivx::Texture2D::create(TEXTURES "Checkerboard.png");
        texture2_ = ivx::Texture2D::create(TEXTURES "ChernoLogo.png");

        shader_->set("u_Texture", 1);
    }

    void on_update(float delta_time) override {
        camera_controller_->on_update(delta_time);
        // draw
        ivx::Renderer::begin_scene(camera_controller_->get_camera());

        shader_->set("u_Color", color_);

        // auto scale = glm::scale(glm::mat4{1.f}, glm::vec3{0.1f, 0.1f, 1.f});
        // auto rotation = glm::rotate(
        //     glm::mat4{1.f}, glm::radians(-45.f), glm::vec3{0.f, 0.f, 1.f});
        // for (int i = 0; i < 10; i++) {
        //     for (int j = 0; j < 10; j++) {
        //         auto translation = glm::translate(
        //             glm::mat4{1.f},
        //             glm::vec3{0.12f * i - 0.5f, 0.12f * j - 0.5f, 0.f});
        //         ivx::Renderer::submit(
        //             shader_, vertex_array_, translation * scale);
        //     }
        // }
        auto scale = glm::scale(glm::mat4{1.f}, glm::vec3{1.f});
        texture_->bind(1);
        ivx::Renderer::submit(shader_, vertex_array_, scale);
        texture2_->bind(1);
        ivx::Renderer::submit(shader_, vertex_array_, scale);

        ivx::Renderer::end_scene();
    }

    void on_render() override {
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Color palette", glm::value_ptr(color_));
        ImGui::End();
    }

    void on_event(ivx::Event &event) override {
        camera_controller_->on_event(event);
    }

  private:
    ivx::Ref<ivx::Shader> shader_;

    ivx::Ref<ivx::VertexArray> vertex_array_;

    ivx::Ref<ivx::OrthoGraphicCameraController> camera_controller_;

    ivx::Ref<ivx::Texture2D> texture_;
    ivx::Ref<ivx::Texture2D> texture2_;

    glm::vec3 color_{1.f};
};

class SandboxApp : public ivx::Application {
  public:
    SandboxApp() {
        // push_layer(new ExampleLayer);
        push_layer(new Sandbox2D);
    }

    ~SandboxApp() {
    }
};

ivx::Application *ivx::create_application() {
    return new SandboxApp;
}
