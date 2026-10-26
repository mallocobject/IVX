#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/renderer/sub_texture2d.h"
#include "ivx/renderer/texture.h"
#include "ivx/scene/scene_camera.h"
#include <entt.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace ivx {

using namespace entt::literals;

struct TagComponent {
    entt::id_type id{"Entity"_hs};
    std::string value{"Entity"};

    TagComponent() = default;
    // avoid TagComponent ... = name
    explicit TagComponent(std::string_view name)
        : id(entt::hashed_string(name.data(), name.size())), value(name) {
    }

    bool operator==(const TagComponent &other) const noexcept {
        return id == other.id;
    }
};

struct TransformComponent {
    glm::vec3 translation{0.f};
    float rotation{0.f};
    glm::vec2 scale{1.f};

    // Derived state: refreshed once per frame by
    // Scene::update_world_transforms() as world = parent_world * load_matrix().
    // Never write it by hand, it is overwritten before rendering.
    glm::mat4 world{1.f};

    // Local matrix (T * R * S) built from the fields above.
    glm::mat4 load_matrix() const {
        return glm::translate(glm::mat4{1.f}, translation) *
               glm::rotate(glm::mat4{1.f},
                           glm::radians(rotation),
                           glm::vec3{0.f, 0.f, 1.f}) *
               glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
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

    // SpriteRendererComponent() = default;
    // SpriteRendererComponent(const Ref<Texture2D> &texture,
    //                         float tiling_factor = 1.f,
    //                         const glm::vec4 &color = glm::vec4{1.f},
    //                         const glm::vec2 &size = glm::vec2{1.f})
    //     : texture(texture), size(size), tiling_factor(tiling_factor),
    //       color(color) {
    // }
    // SpriteRendererComponent(const Ref<SubTexture2D> &sub_texture,
    //                         float tiling_factor = 1.f,
    //                         const glm::vec4 &color = glm::vec4{1.f},
    //                         const glm::vec2 &size = glm::vec2{1.f})
    //     : sub_texture(sub_texture), size(size), tiling_factor(tiling_factor),
    //       color(color) {
    // }
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
