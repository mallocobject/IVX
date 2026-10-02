#pragma once

#ifdef IVX_PLATFORM_WINDOWS


extern invertix::Application* invertix::create_application();

int main(int argc, char* argv[]) {
	auto app = invertix::create_application();
	app->run();
	delete app;
}

#endif