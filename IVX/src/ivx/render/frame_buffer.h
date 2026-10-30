#pragma once

#include "ivx/core/ref_counted.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace ivx {

// What one colour attachment holds.
enum class FrameBufferTextureFormat : std::uint8_t {
    None,
    RGBA8,       // ordinary colour
    RGBA16F,     // HDR colour (nothing uses it yet, kept for post processing)
    RED_INTEGER, // entity ids: R32UI, so no blending and no filtering
};

struct FrameBufferAttachmentSpec {
    FrameBufferTextureFormat format{FrameBufferTextureFormat::RGBA8};
};

struct FrameBufferSpecification {
    uint32_t width{0};
    uint32_t height{0};
    // 4x MSAA by default: the editor viewport is the only framebuffer and its
    // edges are what the user stares at. 1 disables multisampling; a backend
    // that cannot allocate the requested count falls back to the highest one it
    // supports (see the backend).
    uint32_t num_samples{4};
    bool swap_chain_target{false};

    std::vector<FrameBufferAttachmentSpec> attachments;
};

class FrameBuffer : public RefCounted {
  public:
    virtual ~FrameBuffer() = default;

    virtual uint32_t get_color_attachment(uint32_t index = 0) const = 0;

    // Reads one pixel of an integer attachment (an entity id, normally). The
    // origin is bottom-left, like OpenGL's; an out-of-range read answers -1.
    virtual int read_pixel(uint32_t index, uint32_t x, uint32_t y) const = 0;

    virtual void clear(const glm::vec4 &clear_color = glm::vec4{
                           0.1f, 0.1f, 0.1f, 1.f}) = 0;

    virtual const FrameBufferSpecification &get_specification() const = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;

    static Ref<FrameBuffer> create(const FrameBufferSpecification &spec);
};
} // namespace ivx
