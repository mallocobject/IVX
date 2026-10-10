#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "ivx/input.h"
#include "ivx/key_codes.h"
#include "ivx/logger.h"
#include <imgui.h>
#include <iostream>
#include <ivx.h>
#include <memory>

class ExampleLayer : public ivx::Layer {
  public:
    ExampleLayer() : Layer("Example") {
        vertex_array_.reset(ivx::VertexArray::create());
        vertex_array2_.reset(ivx::VertexArray::create());

        float vertices[] = {
            -0.5f, 0.f,  0.f,   0.8f, 0.3f, 0.2f, 1.f,  0.5f, 0.f,  0.f,
            0.2f,  0.3f, 0.3f,  1.f,  0.0f, 0.5f, 0.f,  0.2f, 0.3f, 0.8f,
            1.f,   0.0f, -0.5f, 0.f,  0.2f, 0.3f, 0.8f, 1.f,
        };

        auto vertex_buf =
            std::shared_ptr<ivx::VertexBuffer>(ivx::VertexBuffer::create(
                vertices, sizeof(vertices) / sizeof(float)));

        ivx::BufferLayout bl{{"a_Position", ivx::ShaderDataType::vec3, false},
                             {"a_Color", ivx::ShaderDataType::vec4, false}};
        vertex_buf->set_buffer_layout(bl);

        vertex_array_->add_vertex_buffer(vertex_buf);

        vertex_array2_->add_vertex_buffer(vertex_buf);

        uint32_t indices[] = {
            0,
            1,
            2,
            0,
            3,
            1,
        };

        uint32_t indices2[] = {
            0,
            1,
            2,
        };

        auto index_buf =
            std::shared_ptr<ivx::IndexBuffer>(ivx::IndexBuffer::create(
                indices, sizeof(indices) / sizeof(uint32_t)));

        vertex_array_->set_index_buffer(index_buf);

        auto index_buf2 =
            std::shared_ptr<ivx::IndexBuffer>(ivx::IndexBuffer::create(
                indices2, sizeof(indices2) / sizeof(uint32_t)));

        vertex_array2_->set_index_buffer(index_buf2);

        std::string vs_src = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 mvp;

        out vec4 v_Color;

        void main() {
            v_Color = a_Color;
            gl_Position = mvp * vec4(a_Position, 1.0);
        }
    )";

        std::string fs_src = R"(
        #version 330 core

        layout(location = 0) out vec4 color;

        in vec4 v_Color;

        void main() {
            color = v_Color;
        }
    )";

        shader_.reset(ivx::Shader::create(vs_src, fs_src));

        ivx::RenderCommand::set_clear_color(glm::vec4(0.f));

        camera_.reset(
            new ivx::OrthoGraphicCamera(-1.92f, 1.92f, -1.08f, 1.08f));
    }

    void on_update() override {
        // set
        // z must stay 0: the ortho depth range is [-1, 1], so camera z = 1
        // puts the geometry exactly on the far clip plane and it flickers.
        static glm::vec3 postion{0.f};
        static float degrees = 0.f;
        static constexpr float speed = 1.f;
        static constexpr float delta = 30.f;

        float delta_time = ivx::Timer::tick();

        // IVX_INFO("delta time: {}", delta_time);

        if (ivx::Input::is_key_pressed(IVX_KEY_W)) {
            postion.y += speed * delta_time;
        }
        if (ivx::Input::is_key_pressed(IVX_KEY_S)) {
            postion.y -= speed * delta_time;
        }
        if (ivx::Input::is_key_pressed(IVX_KEY_A)) {
            postion.x -= speed * delta_time;
        }
        if (ivx::Input::is_key_pressed(IVX_KEY_D)) {
            postion.x += speed * delta_time;
        }

        if (ivx::Input::IsMouseButtonPressed(IVX_MOUSE_BUTTON_LEFT)) {
            degrees += delta * delta_time;
        }
        if (ivx::Input::IsMouseButtonPressed(IVX_MOUSE_BUTTON_RIGHT)) {
            degrees -= delta * delta_time;
        }

        camera_->set_position(postion);
        camera_->set_rotation(degrees);

        // draw
        ivx::Renderer::begin_scene(*camera_);

        auto scale = glm::scale(glm::mat4{1.f}, glm::vec3{0.1f, 0.1f, 1.f});
        auto rotation = glm::rotate(
            glm::mat4{1.f}, glm::radians(-45.f), glm::vec3{0.f, 0.f, 1.f});
        for (int j = 0; j < 10; j++) {
            for (int i = 0; i < 10; i++) {
                auto translation = glm::translate(
                    glm::mat4{1.f},
                    glm::vec3{0.11f * i + 0.5f, -0.11f * j + 0.5f, 0.f});
                ivx::Renderer::submit(
                    shader_, vertex_array_, translation * rotation * scale);
            }
        }

        // ivx::Renderer::submit(shader_, vertex_array2_);

        ivx::Renderer::end_scene();
    }

    void on_render() override {
    }

    void on_event(ivx::Event &event) override {
    }

  private:
    std::shared_ptr<ivx::Shader> shader_;

    std::shared_ptr<ivx::VertexArray> vertex_array_;
    std::shared_ptr<ivx::VertexArray> vertex_array2_;

    std::shared_ptr<ivx::OrthoGraphicCamera> camera_;
};

class SandboxApp : public ivx::Application {
  public:
    SandboxApp() {
        push_layer(new ExampleLayer);
    }

    ~SandboxApp() {
    }
};

ivx::Application *ivx::create_application() {
    return new SandboxApp;
}
