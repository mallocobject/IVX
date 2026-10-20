#pragma once

#include "glm/ext/vector_float4.hpp"
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
    static void flush();

    // Primitives
    static void draw_quad(const glm::vec2 &position,
                          const glm::vec2 &size,
                          const glm::vec4 &color);
    static void draw_quad(const glm::vec3 &position,
                          const glm::vec2 &size,
                          const glm::vec4 &color);
    static void draw_quad(const glm::vec2 &position,
                          const glm::vec2 &size,
                          const Ref<Texture2D> &texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_quad(const glm::vec3 &position,
                          const glm::vec2 &size,
                          const Ref<Texture2D> &texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});

    static void draw_rotated_quad(const glm::vec2 &position,
                                  const glm::vec2 &size,
                                  float degrees,
                                  const glm::vec4 &color);
    static void draw_rotated_quad(const glm::vec3 &position,
                                  const glm::vec2 &size,
                                  float degrees,
                                  const glm::vec4 &color);
    static void draw_rotated_quad(const glm::vec2 &position,
                                  const glm::vec2 &size,
                                  float degrees,
                                  const Ref<Texture2D> &texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_rotated_quad(const glm::vec3 &position,
                                  const glm::vec2 &size,
                                  float degrees,
                                  const Ref<Texture2D> &texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});
};
} // namespace ivx