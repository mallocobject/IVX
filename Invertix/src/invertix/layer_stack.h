#pragma once

#include "invertix/core.h"
#include "invertix/layer.h"

#include <vector>

namespace invertix {

	class IVX_API LayerStack
	{
	public:
		using Iterator = std::vector<Layer*>::iterator;

		LayerStack();
		~LayerStack();

		void push_layer(Layer* layer);
		void push_overlay(Layer* overlay);
		void pop_layer(Layer* layer);
		void pop_overlay(Layer* overlay);

		auto begin() { return layers_.begin(); }
		auto end() { return layers_.end(); }
	private:
		std::vector<Layer*> layers_;
		//Iterator layer_insert_;
	};
}