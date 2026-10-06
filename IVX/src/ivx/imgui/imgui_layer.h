#pragma once

#include "ivx/layer.h"

namespace ivx {
	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void on_attach() override;
		void on_detach() override;

		void on_render() override;
		void begin();
		void end();
	private:
		float time_{ 0.f };
	};
}