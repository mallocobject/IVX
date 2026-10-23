#include "ivxpch.h"
#include "imgui_layer.h"
#include "ivx/core/application.h"

#include "ivx/core/logger.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include <backends/imgui_impl_opengl3.h>

#if defined(IVX_WINDOW_SDL3)
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#else
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#endif

#include <imgui.h>

namespace ivx {
ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {
}

void ImGuiLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    Application &app = Application::get();
    void *native_window = app.get_window().get_native_window();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;            // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
#if defined(IVX_WINDOW_SDL3)
    ImGui_ImplSDL3_InitForOpenGL(static_cast<SDL_Window *>(native_window),
                                 app.get_window().get_native_gl_context());
    // SDL3 delivers events through a queue, so every event has to be handed to
    // the backend explicitly (also required for multi-viewport windows).
    app.get_window().set_native_event_callback([](const void *event) {
        ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
    });
#else
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(native_window),
                                 true);
#endif
    ImGui_ImplOpenGL3_Init(nullptr);
}

void ImGuiLayer::on_detach() {
    IVX_PROFILE_FUNCTION();

    ImGui_ImplOpenGL3_Shutdown();
#if defined(IVX_WINDOW_SDL3)
    ImGui_ImplSDL3_Shutdown();
#else
    ImGui_ImplGlfw_Shutdown();
#endif
    ImGui::DestroyContext();
}

void ImGuiLayer::on_event(Event &event) {
    IVX_PROFILE_FUNCTION();

    // If the ImGUI window detaches from the OS GUI, then io.WantCaptureMouse =
    // false, and io.WantCaptureKeyboard = false
    if (event_blocked_) {
        ImGuiIO &io = ImGui::GetIO();
        event.handled |=
            event.is_in_category(EventCategory::kEventCategoryMouse) &
            io.WantCaptureMouse;
        event.handled |=
            event.is_in_category(EventCategory::kEventCategoryKeyboard) &
            io.WantCaptureKeyboard;
        // IVX_CORE_VERBOSE("{}, {}", io.WantCaptureMouse,
        // io.WantCaptureKeyboard);
    }
}

void ImGuiLayer::begin() {
    IVX_PROFILE_FUNCTION();

    ImGuiIO &io = ImGui::GetIO();
    Application &app = Application::get();
    io.DisplaySize = ImVec2(static_cast<float>(app.get_window().get_width()),
                            static_cast<float>(app.get_window().get_height()));

    ImGui_ImplOpenGL3_NewFrame();
#if defined(IVX_WINDOW_SDL3)
    ImGui_ImplSDL3_NewFrame();
#else
    ImGui_ImplGlfw_NewFrame();
#endif
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    IVX_PROFILE_FUNCTION();

    ImGuiIO &io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
#if defined(IVX_WINDOW_SDL3)
        SDL_Window *backup_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_window, backup_context);
#else
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
#endif
    }
}

} // namespace ivx
