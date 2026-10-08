#pragma once

#include "ivx/core.h"
#include "ivx/logger.h"
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace ivx {

enum class ShaderDataType : uint8_t {
    boolean = 0,
    vec,
    vec2,
    vec3,
    vec4,
    ivec2,
    ivec3,
    ivec4,
    mat3,
    mat4,
};

inline constexpr uint32_t size_of(ShaderDataType type) {
    using enum ShaderDataType;
    switch (type) {
    case boolean:
        return 4;
    case vec:
        return 4;
    case vec2:
        return 4 * 2;
    case vec3:
        return 4 * 3;
    case vec4:
        return 4 * 4;
    case ivec2:
        return 4 * 2;
    case ivec3:
        return 4 * 3;
    case ivec4:
        return 4 * 4;
    case mat3:
        return 4 * 3 * 3;
    case mat4:
        return 4 * 4 * 4;
    }

    IVX_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

inline constexpr uint32_t count_of(ShaderDataType type) {
    using enum ShaderDataType;
    switch (type) {
    case boolean:
        return 1;
    case vec:
        return 1;
    case vec2:
        return 2;
    case vec3:
        return 3;
    case vec4:
        return 4;
    case ivec2:
        return 2;
    case ivec3:
        return 3;
    case ivec4:
        return 4;
    case mat3:
        return 3 * 3;
    case mat4:
        return 4 * 4;
    }

    IVX_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

struct BufferElement {
    ShaderDataType type;
    std::string name;
    uint32_t size;
    uint32_t offset;
    bool normalized;

    BufferElement(ShaderDataType type,
                  const std::string &name,
                  bool normalized = false);
};

class BufferLayout {
  public:
    BufferLayout() = default;
    BufferLayout(const std::initializer_list<BufferElement> &elements);
    uint32_t get_stride() const {
        return stride_;
    }
    const std::vector<BufferElement> &get_elements() const {
        return elements_;
    }

    auto begin() {
        return elements_.begin();
    }
    auto end() {
        return elements_.end();
    }

    auto begin() const {
        return elements_.begin();
    }
    auto end() const {
        return elements_.end();
    }

  private:
    void calc_stride();

    std::vector<BufferElement> elements_;
    uint32_t stride_{0};
};

class VertexBuffer {
  public:
    virtual ~VertexBuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    void set_buffer_layout(BufferLayout buf_layout) {
        buf_layout_ = std::move(buf_layout);
    }

    const BufferLayout &get_buffer_layout() const {
        return buf_layout_;
    }

    static VertexBuffer *create(float *vertices, uint32_t count);

  private:
    BufferLayout buf_layout_;
};

class IndexBuffer {
  public:
    virtual ~IndexBuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual uint32_t get_count() const = 0;

    static IndexBuffer *create(uint32_t *indices, uint32_t count);
};
} // namespace ivx