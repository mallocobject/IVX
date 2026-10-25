#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/renderer/camera.h"
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
    std::string tag{"Entity"};

    TagComponent() = default;
    // avoid TagComponent ... = name
    explicit TagComponent(std::string_view name) : tag(name) {
    }
};

struct TransformComponent {
    glm::vec3 translation{0.f};
    float rotation{0.f};
    glm::vec2 scale{1.f};

    // lazy load
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
    float tiling_factor{1.f};
    glm::vec4 color{1.f};
    SpriteRendererComponent() = default;
    SpriteRendererComponent(const Ref<Texture2D> &texture,
                            float tiling_factor = 1.f,
                            const glm::vec4 &color = glm::vec4{1.f})
        : texture(texture), tiling_factor(tiling_factor), color(color) {
    }
    SpriteRendererComponent(const Ref<SubTexture2D> &sub_texture,
                            float tiling_factor = 1.f,
                            const glm::vec4 &color = glm::vec4{1.f})
        : sub_texture(sub_texture), tiling_factor(tiling_factor), color(color) {
    }
};

// HACK
struct CameraComponent {
    SceneCamera camera;
    bool primary{true};
    bool fixed_aspect_ratio{false};
    bool perspective{false};
    // false: orthogonal
    // true: perspective

    CameraComponent(float size,
                    float n,
                    float f,
                    float zoom_level,
                    bool primary = true,
                    bool fix_aspect_ratio = false)
        : camera(size, n, f, zoom_level), primary(primary),
          fixed_aspect_ratio(fix_aspect_ratio) {
    }

    CameraComponent(float fov,
                    float n,
                    float f,
                    bool primary = true,
                    bool fix_aspect_ratio = false)
        : camera(fov, n, f), primary(primary),
          fixed_aspect_ratio(fix_aspect_ratio) {
    }
};

struct NativeScriptComponent;

} // namespace ivx