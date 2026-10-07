#include "ivxpch.h"
#include "layer_stack.h"
#include <ranges>

namespace ivx {

LayerStack::~LayerStack() {
    for (auto &&layer : layers_) {
        delete layer;
    }
}

void LayerStack::push_layer(Layer *layer) {
    layers_.insert(layers_.begin() + layer_insert_idx_, layer);
    layer_insert_idx_++;
    layer->on_attach();
}

void LayerStack::push_overlay(Layer *overlay) {
    layers_.insert(layers_.begin(), overlay);
    overlay->on_attach();
}

void LayerStack::pop_layer(Layer *layer) {
    if (auto it = rg::find(layers_ | vws::take(layer_insert_idx_), layer);
        it != layers_.end()) {
        layer->on_detach();
        layers_.erase(it);
        layer_insert_idx_--;
    }
}

void LayerStack::pop_overlay(Layer *overlay) {
    if (auto it = rg::find(layers_ | vws::drop(layer_insert_idx_), overlay);
        it != layers_.end()) {
        overlay->on_detach();
        layers_.erase(it);
    }
}
} // namespace ivx