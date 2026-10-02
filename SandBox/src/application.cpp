#include<invertix.h>

class Sandbox : public invertix::Application
{
public:
	Sandbox() {

	}

	~Sandbox() {

	}
};

invertix::Application* invertix::create_application() {
	return new Sandbox;
}
