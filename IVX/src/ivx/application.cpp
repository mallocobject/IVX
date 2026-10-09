#include "ivxpch.h"
#include "ivx/application.h"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/input.h"

#include "render/buffer.h"
#include "render/render_command.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

#include "render/ortho_graphic_camera.h"

// #include "backend/opengl/opengl_renderer.h"

namespace ivx {
Application::Application() {
    IVX_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;
    window_ = std::unique_ptr<Window>(Window::create());
    window_->set_event_callback(IVX_BIND_EVENT_FN(Event, on_event));

    imgui_layer_ = new ImGuiLayer;
    push_overlay(imgui_layer_);

    vertex_array_.reset(VertexArray::create());
    vertex_array2_.reset(VertexArray::create());

    float vertices[] = {
        -0.5f, -0.5f, 0.f,  0.8f, 0.3f, 0.2f, 1.f,  0.5f, -0.5f, 0.f,
        0.2f,  0.3f,  0.3f, 1.f,  0.0f, 0.5f, 0.f,  0.2f, 0.3f,  0.8f,
        1.f,   0.0f,  -1.f, 0.f,  0.2f, 0.3f, 0.8f, 1.f,
    };

    auto vertex_buf = std::shared_ptr<VertexBuffer>(
        VertexBuffer::create(vertices, sizeof(vertices) / sizeof(float)));

    BufferLayout bl{{"a_Position", ShaderDataType::vec3, false},
                    {"a_Color", ShaderDataType::vec4, false}};
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

    auto index_buf = std::shared_ptr<IndexBuffer>(
        IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t)));

    vertex_array_->set_index_buffer(index_buf);

    auto index_buf2 = std::shared_ptr<IndexBuffer>(
        IndexBuffer::create(indices2, sizeof(indices2) / sizeof(uint32_t)));

    vertex_array2_->set_index_buffer(index_buf2);

    std::string vs_src = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 vp;

        out vec4 v_Color;

        void main() {
            v_Color = a_Color;
            gl_Position = vp * vec4(a_Position, 1.0);
        }
    )";

    std::string vs_src2 = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        // uniform mat4 vp;

        out vec4 v_Color;

        void main() {
            v_Color = a_Color;
            gl_Position = vec4(a_Position, 1.0);
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

    shader_.reset(Shader::create(vs_src, fs_src));
    shader2_.reset(Shader::create(vs_src2, fs_src));

    RenderCommand::set_clear_color(glm::vec4(0.f));

    OrthoGraphicCamera camera(-1.92f, 1.92f, -1.08f, 1.08f);
    camera.set_position({-0.2f, -0.2f, 0});
    camera.set_degrees(45.f);

    auto vp = camera.get_proj_matrix() * camera.get_view_matrix();

    shader_->set("vp", vp);

    // vertex_array_->unbind();
    // vertex_array2_->unbind();
}

void Application::run() {
    while (running_) {
        // clear
        RenderCommand::clear();

        // draw
        Renderer::begin_scene();

        Renderer::submit(shader_, vertex_array_);

        Renderer::end_scene();

        // update
        for (auto &&layer : layer_stack_) {
            layer->on_update();
        }

        // render
        imgui_layer_->begin();
        for (auto &&layer : layer_stack_) {
            layer->on_render();
        }
        imgui_layer_->end();

        // roll and swap buffer
        window_->on_update();
    }
}

void Application::push_layer(Layer *layer) {
    layer_stack_.push_layer(layer);
}

void Application::push_overlay(Layer *layer) {
    layer_stack_.push_overlay(layer);
}

void Application::on_event(Event &e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch(IVX_BIND_EVENT_FN(WindowCloseEvent, on_window_close));

    // IVX_CORE_TRACE("{}", e);

    for (auto &&layer : layer_stack_ | vws::reverse) {
        if (e.handled) {
            break;
        }
        layer->on_event(e);
    }
}

bool Application::on_window_close(WindowCloseEvent &e) {
    running_ = false;
    return true;
}
} // namespace ivx