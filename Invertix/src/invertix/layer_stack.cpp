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
	}

	void LayerStack::push_overlay(Layer* overlay) {
		layers_.insert(layers_.begin(), overlay);
	}

	void LayerStack::pop_layer(Layer* layer) {
		if (auto it = rg::find(layers_, layer); it != layers_.end())
		{
			layers_.erase(it);
		}
	}

	void LayerStack::pop_overlay(Layer* overlay) {
		pop_layer(overlay);
	}
}