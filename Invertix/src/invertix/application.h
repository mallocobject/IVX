#pragma once

#include"core.h"

namespace invertix {
	class IVX_API Application
	{
	public:
		Application();
		virtual ~Application();

		void run();
	};

	// defined by client
	Application* create_application();
}

