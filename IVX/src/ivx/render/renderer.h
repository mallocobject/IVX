#pragma once

#include "ivx/event/application_event.h"
#include "ivx/render/renderer_api.h"

namespace ivx {

// Engine level entry points only. Per frame drawing belongs to Renderer2D /
// Renderer3D inside a RenderPass; this used to carry a submit() plus a
// SceneData view-projection matrix that nothing ever wrote, which made it look
// like a renderer while doing nothing.
class Renderer {
  public:
    static void init();
    static void shutdown();
    static void on_window_resize(WindowResizeEvent &event);

    static RendererAPI::API get_api() {
        return RendererAPI::get_api();
    }
};
} // namespace ivx
