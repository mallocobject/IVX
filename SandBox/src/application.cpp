#include <invertix.h>
#include <imgui/imgui.h>

class ExampleLayer : public invertix::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void on_update() override
	{
		if (invertix::Input::is_key_pressed(IVX_KEY_TAB)) {
			IVX_TRACE("Tab key is pressed!");
		}
	}

	void on_render() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}

	void on_event(invertix::Event& event) override
	{
		//IVX_TRACE("{}", event);
	}

};

class Sandbox : public invertix::Application
{
public:
	Sandbox() {
		push_layer(new ExampleLayer);
		//push_layer(new invertix::ImGuiLayer);
	}

	~Sandbox() {

	}
};

invertix::Application* invertix::create_application() {
	return new Sandbox;
}
