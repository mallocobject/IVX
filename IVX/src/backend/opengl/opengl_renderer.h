#pragma once

#include "ivx/render/renderer.h"
#include <glm/glm.hpp>

namespace ivx {
class OpenGLRenderer : public Renderer {
  public:
    virtual void set_clear_color(const glm::vec4 &color) override;
    virtual void clear() override;
    virtual void draw() override;
};
} // namespace ivx