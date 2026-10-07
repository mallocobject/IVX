#pragma once

#include "ivx/render/graphics_context.h"

struct GLFWwindow;

namespace ivx {
class OpenGLContext : public GraphicsContext {
  public:
    OpenGLContext(GLFWwindow *window_handle);

    virtual void init() override;
    virtual void swap_buffer() override;
    virtual void clear() override;

  private:
    GLFWwindow *window_handle_;
};
} // namespace ivx
