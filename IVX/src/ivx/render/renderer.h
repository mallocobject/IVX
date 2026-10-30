#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/event/application_event.h"
#include "ivx/render/render_command.h"
#include "ivx/render/renderer_api.h"
#include "ivx/render/shader.h"
#include "ivx/render/vertex_array.h"
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
class Renderer {
  public:
    static void init();
    static void shutdown();
    static void on_window_resize(WindowResizeEvent &event);

    static void submit(const Ref<Shader> &shader,
                       const Ref<VertexArray> &vertex_array,
                       const glm::mat4 &transform = glm::mat4{1.f});
    static RendererAPI::API get_api() {
        return RendererAPI::get_api();
    }

  private:
    struct SceneData {
        glm::mat4 view_proj_matrix;
    };

    static SceneData *scene_data_;
};
} // namespace ivx
