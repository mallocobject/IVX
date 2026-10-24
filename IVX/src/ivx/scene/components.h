#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/renderer/camera.h"
#include "ivx/renderer/sub_texture2d.h"
#include "ivx/renderer/texture.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <string_view>

namespace ivx {

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

    // SRT
    glm::mat4 get_transform() const {
        return glm::translate(glm::mat4{1.f}, translation) *
               glm::rotate(glm::mat4{1.f},
                           glm::radians(rotation),
                           glm::vec3{0.f, 0.f, 1.f}) *
               glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
    }
};

struct SpriteRendererComponent {
    glm::vec4 color{1.f};
    Ref<Texture2D> texture;
    Ref<SubTexture2D> sub_texture;
    float tiling_factor{1.f};
};

struct CameraComponent {
    Ref<OrthoGraphicCamera> camera;
    bool primary{true};
    bool fixed_aspect_ratio{false};
};

struct NativeScriptComponent;

} // namespace ivx