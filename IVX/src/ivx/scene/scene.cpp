#include "ivxpch.h"
#include "scene.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "ivx/core/logger.h"
#include "ivx/renderer/camera.h"
#include "ivx/renderer/renderer2d.h"
#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h"
#include <cstdint>

namespace ivx {
Scene::~Scene() {
    registry_.clear();
}

Entity Scene::create_entity(const std::string &name) {
    Entity entity{registry_.create(), this};
    entity.add_component<TransformComponent>();
    entity.add_component<TagComponent>(name);
    return entity;
}

void Scene::destroy_entity(Entity entity) {
    if (auto *script = entity.try_get_component<NativeScriptComponent>()) {
        if (script->instance && script->destroy) {
            script->instance->on_destroy();
            script->destroy(script);
        }
    }
    registry_.destroy(entity.handle());
}

void Scene::on_update(float dt) {
    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        if (nsc.instance == nullptr) {
            nsc.instance = nsc.instantiate();
            nsc.instance->entity_ = Entity{handle, this};
            nsc.instance->on_create();
        }
    }

    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        if (nsc.enabled) {
            nsc.instance->on_update(dt);
        }
    }
}

void Scene::on_render() {
    glm::mat4 camera_projection{1.f};
    glm::mat4 camera_transform{1.f};
    bool has_main_camera = false;

    for (auto [handle, camera, transform] :
         registry_.view<CameraComponent, TransformComponent>().each()) {
        if (camera.primary) {
            camera_projection = camera.camera.get_proj_matrix();
            camera_transform = transform.load_matrix();
            has_main_camera = true;
            break;
        }
    }

    if (has_main_camera) {
        Renderer2D::begin_scene(camera_projection, camera_transform);
        for (auto [handle, transform, sprite] :
             registry_.view<TransformComponent, SpriteRendererComponent>()
                 .each()) {
            if (sprite.sub_texture) {
                Renderer2D::draw_quad(transform.load_matrix(),
                                      sprite.sub_texture,
                                      sprite.tiling_factor,
                                      sprite.color);
            } else if (sprite.texture) {
                Renderer2D::draw_quad(transform.load_matrix(),
                                      sprite.texture,
                                      sprite.tiling_factor,
                                      sprite.color);
            } else {
                Renderer2D::draw_quad(transform.load_matrix(), sprite.color);
            }
        }
        Renderer2D::end_scene();
    }
}

void Scene::on_viewport_resize(uint32_t width, uint32_t height) {
    for (auto [handle, camera] : registry_.view<CameraComponent>().each()) {
        if (!camera.fixed_aspect_ratio) {
            camera.camera.set_viewport_size(width, height);
        }
    }
}

} // namespace ivx
