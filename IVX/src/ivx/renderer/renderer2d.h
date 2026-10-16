#pragma once

#include "ivx/renderer/camera.h"
#include "texture.h"
#include <glm/glm.hpp>

namespace ivx {
class Renderer2D {
  public:
    static void init();
    static void shutdown();
    static void begin_scene(const Ref<OrthoGraphicCamera> &camera);
    static void end_scene();

    // Primitives
    static void draw_quad(const glm::vec2 &position,
                          const glm::vec2 &size,
                          const glm::vec4 &color);
    static void draw_quad(const glm::vec3 &position,
                          const glm::vec2 &size,
                          const glm::vec4 &color);
    static void draw_quad(const glm::vec2 &position,
                          const glm::vec2 &size,
                          const Ref<Texture2D> &texture);
    static void draw_quad(const glm::vec3 &position,
                          const glm::vec2 &size,
                          const Ref<Texture2D> &texture);
};
} // namespace ivx