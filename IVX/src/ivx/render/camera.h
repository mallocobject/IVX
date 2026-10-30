#pragma once

#include "ivx/core/base.h"
#include "ivx/core/logger.h" // IVX_CORE_ASSERT expands to IVX_CORE_ERROR
#include "ivx/core/ref.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {

// What a renderer needs to know about a view: the two matrices, plus enough of
// the camera to place it in the world (3D lighting needs the eye position,
// shadow and depth passes need the clip planes).
//
// The scene camera does not know its own position - that lives in the entity's
// TransformComponent - so it keeps the default here and whoever renders it is
// expected to use a camera object that does (EditorCamera, or the 3D camera
// later). The defaults are picked so a renderer written against this interface
// still behaves sensibly.
class Camera {
  public:
    virtual ~Camera() = default;

    virtual glm::mat4 get_view_matrix() const {
        IVX_CORE_ASSERT(false, "Camera::get_view_matrix() is not implemented");
        return glm::mat4{1.f};
    }

    virtual glm::mat4 get_proj_matrix() const = 0;

    virtual glm::mat4 get_view_projection() const {
        return get_proj_matrix() * get_view_matrix();
    }

    virtual glm::vec3 get_position() const {
        return glm::vec3{0.f};
    }

    virtual float get_near() const {
        return 0.01f;
    }

    virtual float get_far() const {
        return 10000.f;
    }
};
} // namespace ivx
