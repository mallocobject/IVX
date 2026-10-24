#include "ivxpch.h"
#include "scene.h"
#include "ivx/renderer/camera.h"
#include "ivx/renderer/renderer2d.h"
#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h"

namespace ivx {
Scene::~Scene() {
    registry_.clear();
}

Entity Scene::create_entity(const std::string &name) {
    Entity entity{registry_.create(), this};
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

void Scene::on_update(float delta_time) {
    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        if (nsc.instance == nullptr) {
            nsc.instance = nsc.instantiate();
            nsc.instance->entity_ = Entity{handle, this};
            nsc.instance->on_create();
        }
    }

    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        nsc.instance->on_update(delta_time);
    }
}

void Scene::on_render() {
    // The caller owns the camera: wrap this call in
    // Renderer2D::begin_scene(camera) / end_scene().
    for (auto [handle, transform, sprite] :
         registry_.view<TransformComponent, SpriteRendererComponent>().each()) {
        if (sprite.sub_texture) {
            Renderer2D::draw_quad(transform.get_transform(),
                                  sprite.sub_texture,
                                  sprite.tiling_factor,
                                  sprite.color);
        } else if (sprite.texture) {
            Renderer2D::draw_quad(transform.get_transform(),
                                  sprite.texture,
                                  sprite.tiling_factor,
                                  sprite.color);
        } else {
            Renderer2D::draw_quad(transform.get_transform(), sprite.color);
        }
    }
}

} // namespace ivx