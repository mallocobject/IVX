#pragma once

#include "ivx/core/ref_counted.hpp"
#include <cstdint>
namespace ivx {
struct FrameBufferSpecification {
    uint32_t width{0};
    uint32_t height{0};
    uint32_t num_samples{1};
    bool swap_chain_target{false};
};

class FrameBuffer : public RefCounted {
  public:
    virtual ~FrameBuffer() = default;
    virtual uint32_t get_color_attachment() const = 0;
    virtual const FrameBufferSpecification &get_specification() const = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;

    static Ref<FrameBuffer> create(const FrameBufferSpecification &spec);
};
} // namespace ivx