#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"
#include "sub_texture2d.h"
#include "texture.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class Renderer2D {
  public:
    static void init();
    static void shutdown();
    // `view` is a view matrix (inverse of the camera's world transform), the
    // same convention as the OrthoGraphicCamera overload below.
    static void begin_scene(const glm::mat4 &projection, const glm::mat4 &view);
    static void end_scene();
    static void flush();

    // Primitives
    static void draw_quad(const glm::mat4 &transform,
                          const glm::vec4 &color = glm::vec4{1.f});

    static void draw_quad(const glm::mat4 &transform,
                          const Ref<Texture2D> &texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_quad(const glm::mat4 &transform,
                          const Ref<SubTexture2D> &sub_texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});

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
    static void draw_quad(const glm::vec2 &position,
                          const glm::vec2 &size,
                          const Ref<SubTexture2D> &sub_texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_quad(const glm::vec3 &position,
                          const glm::vec2 &size,
                          const Ref<SubTexture2D> &sub_texture,
                          float tiling_factor = 1.f,
                          const glm::vec4 &tint_color = glm::vec4{1.f});

    static void draw_rotated_quad(const glm::vec2 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const glm::vec4 &color);
    static void draw_rotated_quad(const glm::vec3 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const glm::vec4 &color);
    static void draw_rotated_quad(const glm::vec2 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const Ref<Texture2D> &texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_rotated_quad(const glm::vec3 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const Ref<Texture2D> &texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_rotated_quad(const glm::vec2 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const Ref<SubTexture2D> &sub_texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});
    static void draw_rotated_quad(const glm::vec3 &position,
                                  const glm::vec2 &size,
                                  float rotation,
                                  const Ref<SubTexture2D> &sub_texture,
                                  float tiling_factor = 1.f,
                                  const glm::vec4 &tint_color = glm::vec4{1.f});

    // Stats
    struct Statistics {
        uint32_t num_draw_calls = 0;
        uint32_t num_quads = 0;

        uint32_t get_total_vertex_count() const {
            return num_quads * 4;
        }

        uint32_t get_total_index_count() const {
            return num_quads * 6;
        }
    };

    static void reset_stats();
    static Statistics get_stats();
};
} // namespace ivx
