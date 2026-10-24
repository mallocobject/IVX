#pragma once

#include "ivx/core/ref_counted.hpp"
#include "ivx/scene/components.h"
#include <entt.hpp>
#include <string>

namespace ivx {

class Entity;

class Scene : public RefCounted {
  public:
    Scene() = default;
    ~Scene();

    Entity create_entity(const std::string &name = "Entity");

    void destroy_entity(Entity entity);

    void on_update(float delta_time);
    // Draws every entity that has a TransformComponent +
    // SpriteRendererComponent. The caller owns the camera: wrap it in
    // Renderer2D::begin_scene()/end_scene().
    void on_render();

    void on_viewport_resize(
        float left, float right, float bottom, float top, float n, float f);

    entt::registry &registry() {
        return registry_;
    }
    const entt::registry &registry() const {
        return registry_;
    }

    template <typename... T>
    auto view() {
        return registry_.view<T...>();
    }

  private:
    entt::registry registry_;
};
} // namespace ivx

// Keeps scene.h self-sufficient: Entity's template bodies dereference Scene, so
// Entity has to be defined after Scene. entity.h includes this header back,
// which the include guard turns into a no-op.
#include "ivx/scene/entity.h"