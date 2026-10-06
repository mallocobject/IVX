#include "ivxpch.h"
#include "opengl_context.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <gl/GL.h>

namespace ivx {
	OpenGLContext::OpenGLContext(GLFWwindow* window_handle) : window_handle_(window_handle)
	{
		IVX_CORE_ASSERT(window_handle, "Window handle is null!")
	}

	void OpenGLContext::init()
	{
		glfwMakeContextCurrent(window_handle_);
		int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		IVX_CORE_ASSERT(status, "Failed to initialize Glad!");

		IVX_CORE_INFO("{:-^40}", "OpenGL Info");
		IVX_CORE_INFO("    Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		IVX_CORE_INFO("    Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		IVX_CORE_INFO("    Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	}

	void OpenGLContext::swap_buffer()
	{
		glfwSwapBuffers(window_handle_);
	}

	void OpenGLContext::clear()
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}
