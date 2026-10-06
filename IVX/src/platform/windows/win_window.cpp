#include "ivxpch.h"
#include "platform/windows/win_window.h"
#include "ivx/event/application_event.h"
#include "ivx/event/key_event.h"
#include "ivx/event/mouse_event.h"

#include "backend/opengl/opengl_context.h"

namespace ivx {

	static bool GLFW_Initialized = false;

	static void GLFW_error_callback(int error_code, const char* description) {
		IVX_CORE_ERROR("GLFW Error ({}): {}", error_code, description);
	}

	Window* Window::create(const WindowProps& props)
	{
		return new WinWindow(props);
	}

	WinWindow::WinWindow(const WindowProps& props)
	{
		init(props);
	}

	WinWindow::~WinWindow()
	{
		shutdown();
	}

	void WinWindow::init(const WindowProps& props)
	{
		IVX_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

		data_.title = props.title;
		data_.width = props.width;
		data_.height = props.height;

		if (!GLFW_Initialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			IVX_CORE_ASSERT(success, "Could not intialize GLFW!");
			glfwSetErrorCallback([](int error_code, const char* description) {
				GLFW_error_callback(error_code, description);
				});
			GLFW_Initialized = true;
		}

		window_handle_ = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), data_.title.c_str(), nullptr, nullptr);

		context_ = new OpenGLContext(window_handle_);
		context_->init();

		glfwSetWindowUserPointer(window_handle_, &data_);
		set_v_sync(true);

		// set GLFW callbacks
		glfwSetWindowSizeCallback(window_handle_, [](GLFWwindow* window, int width, int height) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->width = width;
			data->height = height;

			WindowResizeEvent event(width, height);
			data->event_callback(event);
			});

		glfwSetWindowCloseCallback(window_handle_, [](GLFWwindow* window) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data->event_callback(event);
			});

		glfwSetKeyCallback(window_handle_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action) {
			case GLFW_PRESS: {
				KeyPressedEvent event(key, false);
				data->event_callback(event);
				break;
			}
			case GLFW_RELEASE: {
				KeyReleasedEvent event(key);
				data->event_callback(event);
				break;
			}
			case GLFW_REPEAT: {
				KeyPressedEvent event(key, true);
				data->event_callback(event);
				break;
			}
			}
			});

		glfwSetCharCallback(window_handle_, [](GLFWwindow* window, unsigned int codepoint)
			{
				auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				KeyTypedEvent event(codepoint);
				data->event_callback(event);
			});

		glfwSetMouseButtonCallback(window_handle_, [](GLFWwindow* window, int button, int action, int mods) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action) {
			case GLFW_PRESS: {
				MouseButtonPressedEvent event(button);
				data->event_callback(event);
				break;
			}
			case GLFW_RELEASE: {
				MouseButtonReleasedEvent event(button);
				data->event_callback(event);
				break;
			}
			}
			});


		glfwSetScrollCallback(window_handle_, [](GLFWwindow* window, double xoffset, double yoffset) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseScrolledEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
			data->event_callback(event);
			});

		glfwSetCursorPosCallback(window_handle_, [](GLFWwindow* window, double xpos, double ypos) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
			data->event_callback(event);
			});

	}

	void WinWindow::shutdown()
	{
		glfwDestroyWindow(window_handle_);
	}

	void WinWindow::on_update()
	{
		glfwPollEvents();

		context_->swap_buffer();
	}

	void WinWindow::clear()
	{
		context_->clear();
	}

	void WinWindow::set_v_sync(bool enabled)
	{
		if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

		data_.v_sync = enabled;
	}

}
