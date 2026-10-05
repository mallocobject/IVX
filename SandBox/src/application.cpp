#include <invertix.h>

class ExampleLayer : public invertix::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void on_update() override
	{
		IVX_INFO("ExampleLayer::Update");
	}

	void on_event(invertix::Event& event) override
	{
		IVX_TRACE("{}", event);
	}

};

class Sandbox : public invertix::Application
{
public:
	Sandbox() {
		push_layer(new ExampleLayer);
	}

	~Sandbox() {

	}
};

invertix::Application* invertix::create_application() {
	return new Sandbox;
}
