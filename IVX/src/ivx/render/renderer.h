#pragma once

#include "render_command.h"
#include "renderer_api.h"
#include "vertex_array.h"
#include <memory>

namespace ivx {
class Renderer {
  public:
    static void begin_scene();
    static void end_scene();
    static void submit(const std::shared_ptr<VertexArray> &vertex_array);
    static RendererAPI::API get_api() {
        return RendererAPI::get_api();
    }
};
} // namespace ivx