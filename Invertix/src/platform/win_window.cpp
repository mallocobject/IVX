#include "ivx_pch.h"
#include "platform/win_window.h"
#include "invertix/events/application_event.h"
#include "invertix/events/key_event.h"
#include "invertix/events/mouse_event.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace invertix {

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

		window_ = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), data_.title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(window_);
		int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		IVX_CORE_ASSERT(status, "Failed to initailize Glad!");
		glfwSetWindowUserPointer(window_, &data_);
		set_v_sync(true);

		// set GLFW callbacks
		glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->width = width;
			data->height = height;

			WindowResizeEvent event(width, height);
			data->event_callback(event);
			});

		glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			WindowCloseEvent event;
			data->event_callback(event);
			});

		glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
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

		glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
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


		glfwSetScrollCallback(window_, [](GLFWwindow* window, double xoffset, double yoffset) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseScrolledEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
			data->event_callback(event);
			});

		glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double xpos, double ypos) {
			auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			MouseMovedEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
			data->event_callback(event);
			});

	}

	void WinWindow::shutdown()
	{
		glfwDestroyWindow(window_);
	}

	void WinWindow::on_update()
	{
		glfwPollEvents();
		glfwSwapBuffers(window_);
	}

	void WinWindow::clear()
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
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

	bool WinWindow::is_v_sync() const
	{
		return data_.v_sync;
	}

}
