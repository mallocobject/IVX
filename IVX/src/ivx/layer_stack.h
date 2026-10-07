#pragma once

#include "ivx/core.h"
#include "ivx/layer.h"

#include <cstdint>
#include <vector>

namespace ivx {
class LayerStack {
  public:
    // using Iterator = std::vector<Layer *>::iterator;

    LayerStack() = default;
    ~LayerStack();

    void push_layer(Layer *layer);
    void push_overlay(Layer *overlay);
    void pop_layer(Layer *layer);
    void pop_overlay(Layer *overlay);

    auto begin() {
        return layers_.begin();
    }
    auto end() {
        return layers_.end();
    }

  private:
    std::vector<Layer *> layers_;
    uint32_t layer_insert_idx_{0};
};
} // namespace ivx