#pragma once

#include "ivx/core/ref_counted.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace ivx {

// What one colour attachment holds.
enum class FrameBufferColorFormat : std::uint8_t {
    RGBA8,
    RGBA16F,     // linear HDR scene colour
    RED_INTEGER, // entity ids: R32UI, so no blending and no filtering
};

// What the single depth attachment holds. None means there is none, which is
// the normal case for a full screen effect.
enum class FrameBufferDepthFormat : std::uint8_t {
    None,
    Depth24Stencil8, // renderbuffer, not sampleable: the viewport and overlays
    Depth32F,        // texture, sampleable: shadow maps (1 near, 0 far)
};

// A framebuffer is described completely here: the colour attachments in draw
// order, plus at most one depth attachment. Depth is a separate field because
// GL treats it separately - it is never blended, it has its own attachment
// point, and it cannot be filtered like colour.
struct FrameBufferSpecification {
    uint32_t width{0};
    uint32_t height{0};
    // 4x MSAA by default: the editor viewport is the only framebuffer and its
    // edges are what the user stares at. 1 disables multisampling; a backend
    // that cannot allocate the requested count falls back to the highest one it
    // supports (see the backend).
    uint32_t num_samples{4};

    std::vector<FrameBufferColorFormat> color_formats;
    FrameBufferDepthFormat depth_format{FrameBufferDepthFormat::None};
};

class FrameBuffer : public RefCounted {
  public:
    virtual ~FrameBuffer() = default;

    virtual uint32_t get_color_attachment(uint32_t index = 0) const = 0;
    // Sample the resolved attachment after the scene pass has ended.
    virtual void bind_color_attachment(uint32_t slot,
                                       uint32_t index = 0) const = 0;

    // 0 when there is no depth attachment, or when it is a renderbuffer.
    virtual uint32_t get_depth_attachment() const = 0;
    virtual void bind_depth_attachment(uint32_t slot) const = 0;

    // Reads one pixel of an integer attachment (an entity id, normally). The
    // origin is bottom-left, like OpenGL's
    virtual uint32_t
    read_pixel(uint32_t index, uint32_t x, uint32_t y) const = 0;

    // Clears the attachments; the two halves are independent so a depth-only
    // pass can keep the colour that is already there. Integer attachments get 0
    // ("no entity"), depth gets 0.0, the far plane under reverse-Z.
    virtual void
    clear(const glm::vec4 &clear_color = glm::vec4{0.1f, 0.1f, 0.1f, 1.f},
          bool clear_color_attachments = true,
          bool clear_depth = true) = 0;

    virtual const FrameBufferSpecification &get_specification() const = 0;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;

    static Ref<FrameBuffer> create(const FrameBufferSpecification &spec);
};
} // namespace ivx
