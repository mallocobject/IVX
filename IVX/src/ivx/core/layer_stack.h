#pragma once

#include "ivx/core/base.h"
#include "ivx/core/layer.h"
#include "ivx/core/ref.hpp"

#include <cstdint>
#include <vector>

namespace ivx {
class LayerStack {
  public:
    // using Iterator = std::vector<Layer *>::iterator;

    LayerStack() = default;
    ~LayerStack() = default;

    void push_layer(Layer *layer);
    void push_overlay(Layer *overlay);
    void pop_layer(Layer *layer);
    void pop_overlay(Layer *overlay);

    // Detaches every layer (reverse push order) and drops the references, so
    // layers are torn down while the window and the renderer are still alive.
    // Used by ~Application; without it layer destructors would run after the GL
    // context is gone, and the editor's script instances would outlive their
    // module.
    void clear();

    auto begin() {
        return layers_.begin();
    }
    auto end() {
        return layers_.end();
    }

  private:
    std::vector<Ref<Layer>> layers_;
    uint32_t layer_insert_idx_{0};
};
} // namespace ivx
