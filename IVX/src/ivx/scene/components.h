#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/render/sub_texture2d.h"
#include "ivx/render/texture.h"
#include "ivx/scene/scene_camera.h"
#include <entt.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace ivx {

using namespace entt::literals;

namespace detail {
inline glm::mat4 build_rotation(const glm::vec3 &rotation) {
    float yaw = rotation.y;
    float pitch = rotation.x;
    float roll = rotation.z;

    const float cy = std::cos(yaw), sy = std::sin(yaw);
    const float cp = std::cos(pitch), sp = std::sin(pitch);
    const float cr = std::cos(roll), sr = std::sin(roll);

    glm::mat4 R(1.0f);

    // GLM col-main: R[col][row]
    R[0][0] = cy * cr + sy * sp * sr;
    R[0][1] = cp * sr;
    R[0][2] = -sy * cr + cy * sp * sr;

    R[1][0] = -cy * sr + sy * sp * cr;
    R[1][1] = cp * cr;
    R[1][2] = sy * sr + cy * sp * cr;

    R[2][0] = sy * cp;
    R[2][1] = -sp;
    R[2][2] = cy * cp;

    return R;
}
} // namespace detail

struct TagComponent {
    // Display name only. Identity is the entity handle itself: entt::entity is
    // already unique among live entities, so there is nothing to hash or keep
    // in sync. Two entities are allowed to share a name; lookups by name go
    // through Scene::find_by_name().
    std::string value{"Entity"};

    TagComponent() = default;
    // avoid TagComponent ... = name
    explicit TagComponent(std::string_view name) : value(name) {
    }
};

struct IdComponent {
    uint64_t uid{0};
};

struct TransformComponent {
    glm::vec3 translation{0.f};
    glm::vec3 rotation{0.f}; // pitch, yaw, roll
    glm::vec3 scale{1.f};

    // Derived state: refreshed once per frame by
    // Scene::update_world_transforms() as world = parent_world * load_matrix().
    // Never write it by hand, it is overwritten before rendering.
    glm::mat4 world{1.f};

    // Local matrix (T * R * S) built from the fields above.
    glm::mat4 load_matrix() const {
        return glm::translate(glm::mat4{1.f}, translation) *
               detail::build_rotation(rotation) *
               glm::scale(glm::mat4{1.f}, scale);
    }
};

// Parent/child links for the scene hierarchy. Kept separate from
// TransformComponent so moving an entity never touches the tree structure (and
// vice versa). Both directions are maintained by Scene::set_parent().
struct RelationshipComponent {
    entt::entity parent{entt::null};
    std::vector<entt::entity> children;
};

struct SpriteRendererComponent {
    Ref<Texture2D> texture;
    Ref<SubTexture2D> sub_texture;
    // Size of the quad in world units. Kept here (and not in
    // TransformComponent::scale) so that scaling a parent does not multiply the
    // sprite size of its children.
    glm::vec2 size{1.f};
    float tiling_factor{1.f};
    glm::vec4 color{1.f};
};

// Projection parameters live in SceneCamera, the component only adds the
// scene-level flags. Default constructible; `primary` defaults to false so that
// merely adding a camera never takes over rendering.
struct CameraComponent {
    SceneCamera camera;
    bool primary{false};
    bool fixed_aspect_ratio{false};
};

struct NativeScriptComponent;

} // namespace ivx
