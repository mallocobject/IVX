#pragma once

#include "ivx/core/ref_counted.hpp"
#include "ivx/render/camera.h"
#include "ivx/scene/components.h"
#include <entt.hpp>
#include <string>
#include <string_view>
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

    bool rename_entity(Entity entity, std::string_view name);

    // First entity whose TagComponent::value equals `name`, or an invalid
    // Entity. Names are not unique; identity is the handle.
    Entity find_by_name(std::string_view name);

    void set_parent(Entity child, Entity parent);
    std::vector<Entity> get_root_entities();
    std::vector<Entity> get_children(Entity entity);
    bool is_descendant_of(Entity entity, Entity ancestor);

    void on_update(float dt);

    void on_event(Event &event);

    void on_render(Camera &camera);
    void on_render();

    void update_world_transforms();

    void on_viewport_resize(uint32_t width, uint32_t height);

    glm::uvec2 get_viewport_size() const {
        return viewport_size_;
    }

    entt::registry &registry() {
        return registry_;
    }
    const entt::registry &registry() const {
        return registry_;
    }

    const std::string &get_name() const {
        return name_;
    }

    void set_name(std::string name) {
        name_ = std::move(name);
    }

    template <typename... T>
    auto view() {
        return registry_.view<T...>();
    }

  private:
    void update_world_recursive(Entity entity,
                                const glm::mat4 &parent_world,
                                int depth);

    std::string name_{"Untitled"};
    glm::uvec2 viewport_size_{0u, 0u};
    entt::registry registry_;
};
} // namespace ivx

// Keeps scene.h self-sufficient: Entity's template bodies dereference Scene, so
// Entity has to be defined after Scene. entity.h includes this header back,
// which the include guard turns into a no-op.
#include "ivx/scene/entity.h"
