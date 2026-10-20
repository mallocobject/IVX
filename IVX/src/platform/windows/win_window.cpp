#include "ivxpch.h"
#include "win_window.h"
#include "ivx/core/logger.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/application_event.h"
#include "ivx/event/key_event.h"
#include "ivx/event/mouse_event.h"
#include "ivx/renderer/graphics_context.h"
#include "ivx/renderer/renderer.h"
#include <GLFW/glfw3.h>

#include <cstdint>

// #include <GL/GL.h>

namespace ivx {

static uint8_t GLFW_Initialized = 0;

static void GLFW_error_callback(int error_code, const char *description) {
    IVX_CORE_ERROR("GLFW Error ({}): {}", error_code, description);
}

WinWindow::WinWindow(const WindowProps &props) {
    IVX_PROFILE_FUNCTION();

    init(props);
}

WinWindow::~WinWindow() {
    IVX_PROFILE_FUNCTION();

    shutdown();
}

void WinWindow::init(const WindowProps &props) {
    IVX_PROFILE_FUNCTION();

    IVX_CORE_INFO("Creating window {0} ({1}, {2})",
                  props.title,
                  props.width,
                  props.height);

    data_.title = props.title;
    data_.width = props.width;
    data_.height = props.height;

    if (GLFW_Initialized == 0) {
        IVX_PROFILE_SCOPE("glfwInit");
        int success = glfwInit();
        IVX_CORE_ASSERT(success, "Could not intialize GLFW!");
        glfwSetErrorCallback([](int error_code, const char *description) {
            GLFW_error_callback(error_code, description);
        });
    }

    {
        IVX_PROFILE_SCOPE("glfwCreateWindow");
        if (Renderer::get_api() == RendererAPI::API::kOpenGL) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(IVX_CONFIG_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        }
        window_handle_ = glfwCreateWindow(static_cast<int>(props.width),
                                          static_cast<int>(props.height),
                                          data_.title.c_str(),
                                          nullptr,
                                          nullptr);
        GLFW_Initialized++;
    }

    context_ = GraphicsContext::create(window_handle_);
    context_->init();

    glfwSetWindowUserPointer(window_handle_, &data_);
    set_v_sync(true);

    // set GLFW callbacks
    glfwSetWindowSizeCallback(window_handle_,
                              [](GLFWwindow *window, int width, int height) {
                                  //   glViewport(0, 0, width, height);

                                  auto data = reinterpret_cast<WindowData *>(
                                      glfwGetWindowUserPointer(window));
                                  data->width = width;
                                  data->height = height;

                                  WindowResizeEvent event(width, height);
                                  data->event_callback(event);
                              });

    glfwSetWindowCloseCallback(window_handle_, [](GLFWwindow *window) {
        auto data =
            reinterpret_cast<WindowData *>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        data->event_callback(event);
    });

    glfwSetKeyCallback(
        window_handle_,
        [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            auto data = reinterpret_cast<WindowData *>(
                glfwGetWindowUserPointer(window));

            switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(static_cast<KeyCode>(key), false);
                data->event_callback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(static_cast<KeyCode>(key));
                data->event_callback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(static_cast<KeyCode>(key), true);
                data->event_callback(event);
                break;
            }
            }
        });

    glfwSetCharCallback(
        window_handle_, [](GLFWwindow *window, unsigned int codepoint) {
            auto data = reinterpret_cast<WindowData *>(
                glfwGetWindowUserPointer(window));
            KeyTypedEvent event(static_cast<KeyCode>(codepoint));
            data->event_callback(event);
        });

    glfwSetMouseButtonCallback(
        window_handle_,
        [](GLFWwindow *window, int button, int action, int mods) {
            auto data = reinterpret_cast<WindowData *>(
                glfwGetWindowUserPointer(window));

            switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                data->event_callback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                data->event_callback(event);
                break;
            }
            }
        });

    glfwSetScrollCallback(
        window_handle_, [](GLFWwindow *window, double xoffset, double yoffset) {
            auto data = reinterpret_cast<WindowData *>(
                glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xoffset),
                                     static_cast<float>(yoffset));
            data->event_callback(event);
        });

    glfwSetCursorPosCallback(
        window_handle_, [](GLFWwindow *window, double xpos, double ypos) {
            auto data = reinterpret_cast<WindowData *>(
                glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<float>(xpos),
                                  static_cast<float>(ypos));
            data->event_callback(event);
        });
}

void WinWindow::shutdown() {
    IVX_PROFILE_FUNCTION();

    glfwDestroyWindow(window_handle_);

    if (--GLFW_Initialized == 0) {
        IVX_CORE_INFO("Terminating GLFW");
        glfwTerminate();
    }
}

void WinWindow::on_update() {
    IVX_PROFILE_FUNCTION();

    glfwPollEvents();

    context_->swap_buffer();
}

// void WinWindow::clear() {
//     context_->clear();
// }

void WinWindow::set_v_sync(bool enabled) {
    IVX_PROFILE_FUNCTION();

    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    data_.v_sync = enabled;
}

} // namespace ivx
