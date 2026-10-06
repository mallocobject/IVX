#include "ivx_pch.h"
#include "layer_stack.h"

namespace invertix {

	LayerStack::LayerStack() {
	}

	LayerStack::~LayerStack() {
		for (auto&& layer : layers_) {
			delete layer;
		}
	}

	void LayerStack::push_layer(Layer* layer) {
		layers_.push_back(layer);
		layer->on_attach();
	}

	void LayerStack::push_overlay(Layer* overlay) {
		layers_.insert(layers_.begin(), overlay);
		overlay->on_attach();
	}

	void LayerStack::pop_layer(Layer* layer) {
		if (auto it = rg::find(layers_, layer); it != layers_.end())
		{
			layer->on_detach();
			layers_.erase(it);
		}
	}

	void LayerStack::pop_overlay(Layer* overlay) {
		pop_layer(overlay);
	}
}