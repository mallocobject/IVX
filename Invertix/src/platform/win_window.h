#pragma once

#include "invertix/window.h"

struct GLFWwindow;

namespace invertix {

	class WinWindow : public Window
	{
	public:
		WinWindow(const WindowProps& props);
		virtual ~WinWindow();

		void on_update() override;
		void clear() override;

		inline uint32_t get_width() const override { return data_.width; }
		inline uint32_t get_height() const override { return data_.height; }

		// Window attributes
		inline void set_event_callback(const EventCallbackFn& callback) override { data_.event_callback = callback; }
		void set_v_sync(bool enabled) override;
		bool is_v_sync() const override;

	private:
		struct WindowData
		{
			std::string title;
			uint32_t width;
			uint32_t height;
			bool v_sync;

			EventCallbackFn event_callback;
		};

		virtual void init(const WindowProps& props);
		virtual void shutdown();

		GLFWwindow* window_;
		WindowData data_;
	};

}