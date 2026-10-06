#pragma once

extern invertix::Application* invertix::create_application();

int main(int argc, char* argv[]) {
	invertix::set_log_threshold(invertix::LogLevel::TRACE);

	auto app = invertix::create_application();
	app->run();
	delete app;
}
