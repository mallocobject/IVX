#include "ivxpch.h"
#include "imgui_layer.h"
#include "ivx/core/application.h"

#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace ivx {
ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {
}

void ImGuiLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    Application &app = Application::get();
    GLFWwindow *window =
        static_cast<GLFWwindow *>(app.get_window().get_native_window());

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
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(nullptr);
}

void ImGuiLayer::on_detach() {
    IVX_PROFILE_FUNCTION();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::on_event(Event &event) {
    IVX_PROFILE_FUNCTION();

    if (event_blocked_) {
        ImGuiIO &io = ImGui::GetIO();
        event.handled |=
            event.is_in_category(EventCategory::kEventCategoryMouse) &
            io.WantCaptureMouse;
        event.handled |=
            event.is_in_category(EventCategory::kEventCategoryKeyboard) &
            io.WantCaptureKeyboard;
    }
}

void ImGuiLayer::begin() {
    IVX_PROFILE_FUNCTION();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    IVX_PROFILE_FUNCTION();

    ImGuiIO &io = ImGui::GetIO();
    Application &app = Application::get();
    io.DisplaySize = ImVec2(static_cast<float>(app.get_window().get_width()),
                            static_cast<float>(app.get_window().get_height()));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call
    //  glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

} // namespace ivx
