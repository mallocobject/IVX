#include "glm/ext/vector_float4.hpp"
#include "ivx/render/frame_buffer.h"
#include <cstdint>

namespace ivx {
class OpenGLFrameBuffer : public FrameBuffer {
  public:
    OpenGLFrameBuffer(const FrameBufferSpecification &spec);
    virtual ~OpenGLFrameBuffer();

    virtual uint32_t get_color_attachment() const override {
        return color_tex_;
    }

    virtual const FrameBufferSpecification &get_specification() const override {
        return spec_;
    }

    virtual void bind() override;
    virtual void unbind() override;

    virtual void resize(uint32_t width, uint32_t height) override;

  private:
    void invalidate();
    void clear();

    uint32_t fbo_{0};
    uint32_t color_tex_{0};
    uint32_t depth_tex_{0};
    FrameBufferSpecification spec_;

    int previous_fbo_{0};
    int previous_viewport_[4]{0};
};
} // namespace ivx
