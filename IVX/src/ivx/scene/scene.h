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

    // Entities are looked up by their IdComponent::uid, which is a stable id
    // that survives save/load and is what the id buffer of the viewport stores.
    // The handle is only valid for the current session, so uid must never be
    // decoded back into a handle.
    Entity find_by_uid(uint32_t uid) const;

    // Keeps generated uids above `uid`; used after loading a scene so new
    // entities cannot collide with the ones that came from the file.
    void reserve_uid(uint32_t uid);

    void set_parent(Entity child, Entity parent);
    std::vector<Entity> get_root_entities();
    std::vector<Entity> get_children(Entity entity);
    bool is_descendant_of(Entity entity, Entity ancestor);

    void on_update(float dt);

    void on_event(Event &event);

    // Draws into whatever pass is currently open: the pass owns the target, the
    // clear values, the camera block and the pipeline state.
    void render();
    void render_sprites();

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
    // 0 is reserved for "no entity" in the id buffer, so uids start at 1.
    uint32_t next_uid_{1};
    entt::registry registry_;
};
} // namespace ivx

// Keeps scene.h self-sufficient: Entity's template bodies dereference Scene, so
// Entity has to be defined after Scene. entity.h includes this header back,
// which the include guard turns into a no-op.
#include "ivx/scene/entity.h"
