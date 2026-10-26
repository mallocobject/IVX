#pragma once

#include "SDL3/SDL_video.h"
#include "ivx/core/logger.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/renderer/graphics_context.h"
#include <SDL3/SDL.h>
#include <glad/glad.h>

namespace ivx {
namespace detail {
// Shared by every window back end: dump GL strings and enforce the 4.5 floor.
inline void log_opengl_info() {
    IVX_CORE_INFO("{:-^40}", "OpenGL Info");
    IVX_CORE_INFO("    Vendor: {}",
                  reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    IVX_CORE_INFO("    Renderer: {}",
                  reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    IVX_CORE_INFO("    Version: {}",
                  reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    IVX_CORE_INFO("    GLSL: {}",
                  reinterpret_cast<const char *>(
                      glGetString(GL_SHADING_LANGUAGE_VERSION)));

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 5)) {
        IVX_CORE_FATAL("OpenGL 4.5+ is required, but the context is {}.{}. "
                       "Update your graphics driver or lower the requirement.",
                       GLVersion.major,
                       GLVersion.minor);
        IVX_CORE_ASSERT(false, "Unsupported OpenGL version!");
    }
}

inline void log_sdl_info() {
    const int version = SDL_GetVersion();

    IVX_CORE_INFO("{:-^40}", "SDL Info");
    IVX_CORE_INFO("    SDL: {}.{}.{}",
                  SDL_VERSIONNUM_MAJOR(version),
                  SDL_VERSIONNUM_MINOR(version),
                  SDL_VERSIONNUM_MICRO(version));
    IVX_CORE_INFO("    Revision: {}", SDL_GetRevision());
    IVX_CORE_INFO("    Platform: {}", SDL_GetPlatform());
    IVX_CORE_INFO("    Video driver: {}",
                  SDL_GetCurrentVideoDriver() != nullptr
                      ? SDL_GetCurrentVideoDriver()
                      : "<none>");
    IVX_CORE_INFO("    Compiled against: {}.{}.{}",
                  SDL_MAJOR_VERSION,
                  SDL_MINOR_VERSION,
                  SDL_MICRO_VERSION);
}
} // namespace detail

template <typename T>
class OpenGLContext : public GraphicsContext {
  public:
    explicit OpenGLContext(T *window_handle) {
        IVX_CORE_ASSERT(window_handle, "Window handle is null!");
    }

    void init() override {
        IVX_CORE_ASSERT(
            false,
            "OpenGLContext::init() not implemented for this window type!");
    }

    void swap_buffer() override {
        IVX_CORE_ASSERT(false,
                        "OpenGLContext::swap_buffer() not implemented for this "
                        "window type!");
    }
};

template <>
class OpenGLContext<SDL_Window> : public GraphicsContext {
  public:
    explicit OpenGLContext(SDL_Window *window_handle) : window_(window_handle) {
        IVX_CORE_ASSERT(window_handle, "Window handle is null!");
    }

    void init() override {
        IVX_PROFILE_FUNCTION();

        gl_context_ = SDL_GL_CreateContext(window_);
        IVX_CORE_ASSERT(
            gl_context_, "SDL_GL_CreateContext failed: {}", SDL_GetError());
        SDL_GL_MakeCurrent(window_, gl_context_);
        int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
        IVX_CORE_ASSERT(status, "Failed to initialize Glad!");

        detail::log_sdl_info();
        detail::log_opengl_info();
    }

    void swap_buffer() override {
        IVX_PROFILE_FUNCTION();

        SDL_GL_SwapWindow(window_);
    }

    ~OpenGLContext() override {
        IVX_PROFILE_FUNCTION();

        if (gl_context_ != nullptr) {
            SDL_GL_DestroyContext(gl_context_);
            gl_context_ = nullptr;
        }
    }

    void *get_native_context() override {
        return gl_context_;
    }

  private:
    SDL_Window *window_{nullptr};
    SDL_GLContext gl_context_{nullptr};
};
} // namespace ivx
