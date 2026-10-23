#pragma once

#include "ivx/core/base.h"

#if defined(IVX_PLATFORM_WINDOWS)

extern ivx::Application *ivx::create_application();

int main(int argc, char *argv[]) {
    ivx::set_log_threshold(ivx::LogLevel::VERBOSE);
    IVX_PROFILE_BEGIN_SESSION("startup", "ivx_profile-startup.json");
    auto app = ivx::create_application();
    IVX_PROFILE_END_SESSION();

    IVX_PROFILE_BEGIN_SESSION("runtime", "ivx_profile-runtime.json");
    app->run();
    IVX_PROFILE_END_SESSION();

    IVX_PROFILE_BEGIN_SESSION("shutdown", "ivx_profile-shutdown.json");
    delete app;
    IVX_PROFILE_END_SESSION();
}

#endif
