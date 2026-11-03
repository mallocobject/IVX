#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/render/light.h"
#include "ivx/render/material.h"
#include "ivx/render/mesh.h"
#include "ivx/render/sub_texture2d.h"
#include "ivx/render/texture.h"
#include "ivx/scene/scene_camera.h"
#include "ivx/utils/math.h"
#include <entt.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace ivx {

using namespace entt::literals;

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
    uint32_t uid{0};
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
               rotation_matrix_from_euler(rotation) *
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

struct MeshRendererComponent {
    Ref<Mesh> mesh;
    Material material;
    bool visible{true};
};

// Position and direction come from the entity's transform: a directional light
// shines along its -Z axis, a point light sits at its translation.
struct LightComponent {
    LightSettings settings;
};

struct NativeScriptComponent;

// Every component type that may cross the script boundary. Scripts call
// get_component<T>(), which instantiates entt's lookup inside the module DLL,
// so the layout of each of these has to match between the module and the
// engine. Listed once, here, so a new component cannot silently drop out of the
// ABI fingerprint (see ivx/script/script_abi.h).
//
// RelationshipComponent is included even though it holds entt handles: the
// serializer and the hierarchy data sit next to the other components in the
// same storage.
#define IVX_ABI_COMPONENT_LIST(X)                                              \
    X(TagComponent)                                                            \
    X(IdComponent)                                                             \
    X(TransformComponent)                                                      \
    X(RelationshipComponent)                                                   \
    X(SpriteRendererComponent)                                                 \
    X(MeshRendererComponent)                                                   \
    X(LightComponent)                                                          \
    X(CameraComponent)                                                         \
    X(NativeScriptComponent)

} // namespace ivx
