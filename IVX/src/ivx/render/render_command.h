#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class RenderCommand {
  public:
    // The state a pass can change. It is tracked here so RenderPass can put
    // back what it found instead of guessing a default: with several passes per
    // frame, the previous values are the only correct ones to restore.
    struct State {
        bool depth_test{true};
        bool depth_write{true};
        bool blend{true};
        bool cull_faces{true};
    };

    static void init();
    static void set_viewport(int x, int y, int width, int height);
    static void set_clear_color(const glm::vec4 &color);
    static void set_depth_test(bool enabled = true);
    static void set_depth_write(bool enabled = true);
    static void set_blend(bool enabled = true);
    static void set_cull_faces(bool enabled = true);
    static const State &state() {
        return state_;
    }
    static void clear(bool clear_color = true, bool clear_depth = true);
    static void draw(const Ref<VertexArray> &vertex_array);
    static void draw(const Ref<VertexArray> &vertex_array,
                     uint32_t num_indices);

  private:
    static Ref<RendererAPI> renderer_api_;
    static State state_;
};
} // namespace ivx
