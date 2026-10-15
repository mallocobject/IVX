#pragma once

extern ivx::Application *ivx::create_application();

int main(int argc, char *argv[]) {
    ivx::set_log_threshold(ivx::LogLevel::TRACE);

    auto app = ivx::create_application();
    app->run();
    delete app;
}
