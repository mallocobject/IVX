#include "example_layer.h"
#include "sandbox2d.h"
#include <imgui.h>
#include <ivx.h>

#include <ivx/core/ivx_main.h>

class SandboxApp : public ivx::Application {
  public:
    SandboxApp() {
        // push_layer(new ExampleLayer);
        push_layer(new Sandbox2D);
    }

    ~SandboxApp() {
    }
};

ivx::Application *ivx::create_application() {
    return new SandboxApp;
}
