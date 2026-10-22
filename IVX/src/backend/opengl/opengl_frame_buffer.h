#include "glm/ext/vector_float4.hpp"
#include "ivx/renderer/frame_buffer.h"
#include <cstdint>

namespace ivx {
class OpenGLFrameBuffer : public FrameBuffer {
  public:
    OpenGLFrameBuffer(const FrameBufferSpecification &spec);
    virtual ~OpenGLFrameBuffer();

    void invalidate();

    virtual uint32_t get_color_attachment() const override {
        return color_tex_;
    }

    virtual const FrameBufferSpecification &get_specification() const override {
        return spec_;
    }

    virtual void bind() override;
    virtual void unbind() override;

  private:
    uint32_t fbo_;
    uint32_t color_tex_;
    uint32_t depth_tex_;
    FrameBufferSpecification spec_;
};
} // namespace ivx