#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "ivx/core/ref.hpp"
#include "ortho_graphic_camera.h"
#include "render_command.h"
#include "renderer_api.h"
#include "shader.h"
#include "vertex_array.h"
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
class Renderer {
  public:
    static void init();

    static void begin_scene(const Ref<OrthoGraphicCamera> &camera);
    static void end_scene();
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