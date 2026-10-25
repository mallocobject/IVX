#pragma once

#include "ivx/core/ref_counted.hpp"
#include "ivx/scene/components.h"
#include <entt.hpp>
#include <string>
#include <vector>

namespace ivx {

class Entity;
class Event;

class Scene : public RefCounted {
  public:
    Scene() = default;
    ~Scene();

    Entity create_entity(const std::string &name = "Entity");
    // Same as create_entity(), then attaches the new entity under `parent`.
    Entity create_child(Entity parent, const std::string &name = "Entity");

    // Destroys the entity and, recursively, everything below it.
    void destroy_entity(Entity entity);

    // ---- hierarchy ----
    // Reparents `child`. Passing an invalid `parent` makes it a root node.
    // Rejects self-parenting and anything that would create a cycle.
    void set_parent(Entity child, Entity parent);
    std::vector<Entity> get_root_entities();
    std::vector<Entity> get_children(Entity entity);
    bool is_descendant_of(Entity entity, Entity ancestor);

    void on_update(float dt);
    // Forwards an engine event to every enabled script's on_key_* / on_mouse_*
    // hooks. Call it from the layer's on_event(), so scripts see the same
    // events the rest of the app sees (and nothing while ImGui has the
    // keyboard).
    void on_event(Event &event);
    // Draws every entity that has a TransformComponent +
    // SpriteRendererComponent. The caller owns the camera: wrap it in
    // Renderer2D::begin_scene()/end_scene().
    void on_render();

    // Refreshes TransformComponent::world for the whole hierarchy, parents
    // before children (world = parent_world * local). on_render() calls this,
    // call it yourself if you need world transforms outside of rendering.
    void update_world_transforms();

    void on_viewport_resize(uint32_t width, uint32_t height);

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
    void update_world_recursive(Entity entity,
                                const glm::mat4 &parent_world,
                                int depth);

    entt::registry registry_;
};
} // namespace ivx

// Keeps scene.h self-sufficient: Entity's template bodies dereference Scene, so
// Entity has to be defined after Scene. entity.h includes this header back,
// which the include guard turns into a no-op.
#include "ivx/scene/entity.h"