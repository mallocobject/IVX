#pragma once

#include "ivx/core/base.h"
#include "ivx/core/logger.h" // IVX_CORE_ASSERT expands to IVX_CORE_ERROR
#include "ivx/core/ref.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {
class Camera {
  public:
    virtual ~Camera() = default;
    virtual glm::mat4 get_view_matrix() const {
        IVX_CORE_ASSERT(false, "Have been not inplement!");
        return {};
    }

    virtual glm::mat4 get_proj_matrix() const = 0;
};
} // namespace ivx
