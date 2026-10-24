#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/renderer/camera.h"
#include "ivx/renderer/sub_texture2d.h"
#include "ivx/renderer/texture.h"
#include <entt.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>

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

    auto operator==(const TagComponent &other) const noexcept {
        return id == other.id;
    }

    auto operator<=>(const TagComponent &other) const noexcept {
        return id <=> other.id;
    }
};

struct TransformComponent {
    // glm::vec3 translation{0.f};
    // float rotation{0.f};
    // glm::vec2 scale{1.f};

    // // SRT
    // glm::mat4 get_transform() const {
    //     return glm::translate(glm::mat4{1.f}, translation) *
    //            glm::rotate(glm::mat4{1.f},
    //                        glm::radians(rotation),
    //                        glm::vec3{0.f, 0.f, 1.f}) *
    //            glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
    // }

    glm::mat4 transform{1.f};

    operator glm::mat4 &() {
        return transform;
    }

    operator const glm::mat4 &() const {
        return transform;
    }
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
    glm::mat4 projection{1.f};
    bool primary{false};
    bool fixed_aspect_ratio{false};

    CameraComponent() = default;
    CameraComponent(const glm::mat4 &projection,
                    bool primary = false,
                    bool fix_aspect_ratio = false)
        : projection(projection), primary(primary),
          fixed_aspect_ratio(fix_aspect_ratio) {
    }
};

struct NativeScriptComponent;

} // namespace ivx