#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <cstdint>

namespace ivx {

// A uniform buffer bound to a fixed binding point. Every program that declares
// the same layout(std140, binding = N) block reads it, so data that is constant
// for a whole pass (the camera matrices, later the lights) is uploaded once
// instead of once per shader.
class UniformBuffer : public RefCounted {
  public:
    virtual ~UniformBuffer() = default;

    virtual void
    set_data(const void *data, uint32_t size, uint32_t offset = 0) = 0;

    // Points `binding` at this buffer. Several passes can own camera data at
    // the same time, so the active one has to be re-bound at the start of every
    // pass.
    virtual void bind(uint32_t binding) = 0;

    static Ref<UniformBuffer> create(uint32_t size, uint32_t binding);
};
} // namespace ivx
