#include "ivxpch.h"
#include "layer_stack.h"
#include "ivx/core/layer.h"
#include <ranges>

namespace ivx {

void LayerStack::push_layer(Layer *layer) {
    layers_.emplace(layers_.begin() + layer_insert_idx_, layer);
    layer_insert_idx_++;
}

void LayerStack::push_overlay(Layer *overlay) {
    layers_.emplace(layers_.begin(), overlay);
}

void LayerStack::pop_layer(Layer *layer) {
    auto view = layers_ | vws::take(layer_insert_idx_);
    if (auto it = rg::find(view, layer, [](auto &&e) { return e.get(); });
        it != rg::end(view)) {
        layer->on_detach();
        layers_.erase(it);
        layer_insert_idx_--;
    }
}

void LayerStack::pop_overlay(Layer *overlay) {
    auto view = layers_ | vws::drop(layer_insert_idx_);
    if (auto it = rg::find(view, overlay, [](auto &&e) { return e.get(); });
        it != rg::end(view)) {
        overlay->on_detach();
        layers_.erase(it);
    }
}

} // namespace ivx
