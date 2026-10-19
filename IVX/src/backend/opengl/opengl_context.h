#pragma once

#include "ivx/renderer/graphics_context.h"
#include <GLFW/glfw3.h>

namespace ivx {
class OpenGLContext : public GraphicsContext {
  public:
    OpenGLContext(GLFWwindow *window_handle);

    virtual void init() override;
    virtual void swap_buffer() override;
    // virtual void clear() override;

  private:
    GLFWwindow *window_handle_;
};
} // namespace ivx
