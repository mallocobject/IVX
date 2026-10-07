#include "ivxpch.h"
#include "ivx/application.h"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/input.h"

#include "render/buffer.h"
#include "render/shader.h"
#include <cstdint>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

#include "backend/opengl/opengl_renderer.h"

namespace ivx {
Application::Application() {
    IVX_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;
    window_ = std::unique_ptr<Window>(Window::create());
    window_->set_event_callback(IVX_BIND_EVENT_FN(Event, on_event));

    imgui_layer_ = new ImGuiLayer;
    push_overlay(imgui_layer_);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // glGenBuffers(1, &VBO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float vertices[3 * 3] = {
        -0.5f, -0.5f, 0.f, 0.5f, -0.5f, 0.f, 0.0f, 0.5f, 0.f};

    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
    // GL_STATIC_DRAW);

    vertex_buf_.reset(
        VertexBuffer::create(vertices, sizeof(vertices) / sizeof(float)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        reinterpret_cast<const void *>(static_cast<std::uintptr_t>(0)));

    BufferLayout bl{{"a_Position", ShaderDataType::vec3},
                    {"o_color", ShaderDataType::vec4}};

    // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    uint32_t indices[3] = {0, 1, 2};
    // glBufferData(
    //     GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    index_buf_.reset(
        IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t)));

    std::string vs_src = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;

        out vec3 v_Position;

        void main() {
            v_Position = a_Position;
            gl_Position = vec4(a_Position, 1.0);
        }
    )";

    std::string fs_src = R"(
        #version 330 core

        layout(location = 0) out vec4 color;

        in vec3 v_Position;

        void main() {
            color = vec4(v_Position * 0.5 + 0.5, 1.0);
        }
    )";

    shader_.reset(Shader::create(vs_src, fs_src));
    renderer_.reset(new OpenGLRenderer);
    renderer_->set_clear_color(glm::vec4(0.f));
}

void Application::run() {
    while (running_) {
        // clear
        renderer_->clear();

        // draw
        shader_->bind();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        // glDrawArrays(GL_TRIANGLES, 0, 3);

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