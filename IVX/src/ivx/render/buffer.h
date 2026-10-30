#pragma once

#include "ivx/core/base.h"
#include "ivx/core/logger.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
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
    ivec,
    ivec2,
    ivec3,
    ivec4,
    uvec,
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
    case ivec:
        return 4;
    case ivec2:
        return 4 * 2;
    case ivec3:
        return 4 * 3;
    case ivec4:
        return 4 * 4;
    case uvec:
        return 4;
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
    case ivec:
        return 1;
    case ivec2:
        return 2;
    case ivec3:
        return 3;
    case ivec4:
        return 4;
    case uvec:
        return 1;
    case mat3:
        return 3;
    case mat4:
        return 4;
    }

    IVX_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    uint32_t offset;
    bool normalized;

    BufferElement(const std::string &name,
                  ShaderDataType type,
                  bool normalized = false);
};

class BufferLayout : public RefCounted {
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

class VertexBuffer : public RefCounted {
  public:
    virtual ~VertexBuffer() = default;

    virtual void set_data(const void *data, uint32_t size) = 0;

    virtual uint32_t get_id() const = 0;

    void set_buffer_layout(const Ref<BufferLayout> &buf_layout) {
        buf_layout_ = buf_layout;
    }

    const Ref<BufferLayout> get_buffer_layout() const {
        return buf_layout_;
    }

    static Ref<VertexBuffer> create(uint32_t size);
    static Ref<VertexBuffer> create(float *vertices, uint32_t count);

  private:
    Ref<BufferLayout> buf_layout_;
};

class IndexBuffer : public RefCounted {
  public:
    virtual ~IndexBuffer() = default;

    virtual uint32_t get_count() const = 0;

    virtual uint32_t get_id() const = 0;

    static Ref<IndexBuffer> create(uint32_t *indices, uint32_t count);
};
} // namespace ivx
