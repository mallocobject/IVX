#pragma once

#include "ivx/render/uniform_buffer.h"

namespace ivx {
class OpenGLUniformBuffer : public UniformBuffer {
  public:
    virtual ~OpenGLUniformBuffer();
    OpenGLUniformBuffer(uint32_t size, uint32_t binding);

    virtual void
    set_data(const void *data, uint32_t size, uint32_t offset = 0) override;
    virtual void bind(uint32_t binding) override;

  private:
    uint32_t ubo_{0};
};
} // namespace ivx
