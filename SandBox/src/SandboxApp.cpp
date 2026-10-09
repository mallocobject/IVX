#include <imgui.h>
#include <ivx.h>

class ExampleLayer : public ivx::Layer {
  public:
    ExampleLayer() : Layer("Example") {
    }

    void on_update() override {
        if (ivx::Input::is_key_pressed(IVX_KEY_TAB)) {
            IVX_TRACE("Tab key is pressed!");
        }
    }

    void on_render() override {
        ImGui::Begin("Test");
        ImGui::Text("Hello World!");
        ImGui::End();
    }

    void on_event(ivx::Event &event) override {
        // IVX_TRACE("{}", event);
    }
};

class SandboxApp : public ivx::Application {
  public:
    SandboxApp() {
        push_layer(new ExampleLayer);
    }

    ~SandboxApp() {
    }
};

ivx::Application *ivx::create_application() {
    return new SandboxApp;
}
