#pragma once

#include "ivx/render/shader.h"
#include "ivx/render/vertex_array.h"
#include "ivx/scene/environment.h"

namespace ivx {
class Camera;

class GroundRenderer {
  public:
    void render(const Camera &camera, const GroundSettings &settings);

  private:
    Ref<Shader> shader_;
    Ref<VertexArray> triangle_;
};
} // namespace ivx
