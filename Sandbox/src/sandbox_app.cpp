#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/input.h"
#include "ivx/key_codes.h"
#include "ivx/logger.h"
#include "ivx/render/buffer.h"
#include "ivx/render/texture.h"
#include <imgui.h>
#include <iostream>
#include <ivx.h>

class ExampleLayer : public ivx::Layer {
  public:
    ExampleLayer() : Layer("Example") {
        vertex_array_ = ivx::VertexArray::create();

        float vertices[5 * 4] = {
            -0.5f, -0.5f, 0.f, 0.f, 0.f, 0.5f,  -0.5f, 0.f, 1.f, 0.f,
            0.5f,  0.5f,  0.f, 1.f, 1.f, -0.5f, 0.5f,  0.f, 0.f, 1.f,
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

        std::string vs_src = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec2 a_Texcoord;

        out vec2 v_Texcoord;

        uniform mat4 mvp;

        void main() {
            v_Texcoord = a_Texcoord;
            gl_Position = mvp * vec4(a_Position, 1.0);
        }
    )";

        std::string fs_src = R"(
        #version 330 core

        layout(location = 0) out vec4 color;

        in vec2 v_Texcoord;

        uniform sampler2D u_Texture;

        void main() {
            color = texture(u_Texture, v_Texcoord);
        }
    )";

        shader_ = ivx::Shader::create(vs_src, fs_src);

        ivx::RenderCommand::set_clear_color(glm::vec4(0.f));

        camera_ = ivx::OrthoGraphicCamera::create(-1.92f, 1.92f, -1.08f, 1.08f);

        texture_ = ivx::Texture2D::create(TEXTURES "Checkerboard.png");
        texture_->bind(1);
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
        ivx::Renderer::begin_scene(camera_);

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
        shader_->set("u_Texture", 1);
        auto scale = glm::scale(glm::mat4{1.f}, glm::vec3{2.f});
        ivx::Renderer::submit(shader_, vertex_array_, scale);

        ivx::Renderer::end_scene();
    }

    void on_render() override {
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("Color palette", glm::value_ptr(color_));
        ImGui::End();
    }

    void on_event(ivx::Event &event) override {
    }

  private:
    ivx::Ref<ivx::Shader> shader_;

    ivx::Ref<ivx::VertexArray> vertex_array_;

    ivx::Ref<ivx::OrthoGraphicCamera> camera_;

    ivx::Ref<ivx::Texture2D> texture_;

    glm::vec3 color_{1.f};
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
