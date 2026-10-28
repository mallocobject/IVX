#include "ivxpch.h"
#include "scene.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "ivx/core/logger.h"
#include "ivx/event/key_event.h"
#include "ivx/event/mouse_event.h"
#include "ivx/render/renderer2d.h"
#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace ivx {
namespace {
// Scripts only see events while they are enabled and instantiated.
template <typename Fn>
void for_each_enabled_script(entt::registry &registry, Fn &&fn) {
    for (auto [handle, nsc] : registry.view<NativeScriptComponent>().each()) {
        if (nsc.enabled && nsc.instance != nullptr) {
            fn(*nsc.instance);
        }
    }
}

// Calls on_destroy() and releases the script instance, if there is one.
//
// Ownership note: the instance, its vtable and `destroy` all live in the script
// module (a DLL). Whoever unloads that module has to release every instance
// first, otherwise this jumps into freed code. ScriptHost does exactly that in
// on_detach() and in its destructor.
void release_script(entt::registry &registry, entt::entity handle) {
    auto *script = registry.try_get<NativeScriptComponent>(handle);
    if (script == nullptr) {
        return;
    }
    if (script->instance != nullptr) {
        script->instance->on_destroy();
    }
    if (script->destroy != nullptr) {
        script->destroy(script);
    }
}
} // namespace

Scene::~Scene() {
    // Scripts are owned by the entities, so they have to be released before the
    // registry (which only destroys the components) goes away.
    for (auto [handle, script] :
         registry_.view<NativeScriptComponent>().each()) {
        release_script(registry_, handle);
    }
    registry_.clear();
}

Entity Scene::create_entity(const std::string &name) {
    Entity entity{registry_.create(), this};
    entity.add_component<TransformComponent>();
    entity.add_component<RelationshipComponent>();
    entity.add_component<TagComponent>(name);
    entity.add_component<IdComponent>(
        static_cast<uint64_t>(entity.handle())); // HACK
    return entity;
}

Entity Scene::create_child(Entity parent, const std::string &name) {
    Entity child = create_entity(name);
    set_parent(child, parent);
    return child;
}

void Scene::destroy_entity(Entity entity) {
    if (!entity || !registry_.valid(entity.handle())) {
        return;
    }

    // Children first: their handles have to be captured before the parent (and
    // therefore its RelationshipComponent) is destroyed.
    if (auto *rel = entity.try_get_component<RelationshipComponent>()) {
        auto children = rel->children;
        for (auto child_handle : children) {
            if (registry_.valid(child_handle)) {
                destroy_entity(Entity{child_handle, this});
            }
        }
    }

    // Detach from the parent so its children list does not keep a stale handle.
    if (auto *rel = entity.try_get_component<RelationshipComponent>();
        rel != nullptr && rel->parent != entt::null) {
        if (auto *parent_rel =
                registry_.try_get<RelationshipComponent>(rel->parent);
            parent_rel != nullptr) {
            auto &siblings = parent_rel->children;
            std::erase(siblings, entity.handle());
        }
    }

    // No tag bookkeeping: the entity handle is the identity, so nothing has to
    // be removed from a name index here.
    release_script(registry_, entity.handle());
    registry_.destroy(entity.handle());
}

Entity Scene::find_by_name(std::string_view name) {
    for (auto [handle, tag] : registry_.view<TagComponent>().each()) {
        if (tag.value == name) {
            return Entity{handle, this};
        }
    }
    return {};
}

bool Scene::rename_entity(Entity entity, std::string_view name) {
    auto &tag = entity.get_component<TagComponent>();
    if (name.empty()) {
        return false;
    }
    tag.value = name;
    return true;
}

void Scene::set_parent(Entity child, Entity parent) {
    if (!child || child == parent) {
        return;
    }
    if (parent && is_descendant_of(parent, child)) {
        return; // would create a cycle
    }

    auto *child_rel = child.try_get_component<RelationshipComponent>();
    if (child_rel == nullptr) {
        child_rel = &child.add_component<RelationshipComponent>();
    }

    // Detach from the current parent.
    if (child_rel->parent != entt::null) {
        if (auto *old_rel =
                registry_.try_get<RelationshipComponent>(child_rel->parent);
            old_rel != nullptr) {
            std::erase(old_rel->children, child.handle());
        }
    }

    child_rel->parent = parent ? parent.handle() : entt::null;

    if (parent) {
        auto *parent_rel = parent.try_get_component<RelationshipComponent>();
        if (parent_rel == nullptr) {
            parent_rel = &parent.add_component<RelationshipComponent>();
        }
        if (std::ranges::find(parent_rel->children, child.handle()) ==
            parent_rel->children.end()) {
            parent_rel->children.push_back(child.handle());
        }
    }
}

std::vector<Entity> Scene::get_root_entities() {
    std::vector<Entity> roots;
    for (auto [handle, rel] : registry_.view<RelationshipComponent>().each()) {
        if (rel.parent == entt::null) {
            roots.emplace_back(handle, this);
        }
    }
    return roots;
}

std::vector<Entity> Scene::get_children(Entity entity) {
    std::vector<Entity> children;
    if (!entity) {
        return children;
    }
    if (const auto *rel = entity.try_get_component<RelationshipComponent>()) {
        children.reserve(rel->children.size());
        for (auto handle : rel->children) {
            if (registry_.valid(handle)) {
                children.emplace_back(handle, this);
            }
        }
    }
    return children;
}

bool Scene::is_descendant_of(Entity entity, Entity ancestor) {
    if (!entity || !ancestor) {
        return false;
    }
    const auto *rel = entity.try_get_component<RelationshipComponent>();
    while (rel != nullptr && rel->parent != entt::null) {
        if (rel->parent == ancestor.handle()) {
            return true;
        }
        rel = registry_.try_get<RelationshipComponent>(rel->parent);
    }
    return false;
}

void Scene::on_update(float dt) {
    // Unbound entries are legal (the editor creates them and the user picks a
    // script later), so `instantiate` must be checked before being called -- a
    // null function pointer is an instant crash, not an exception.
    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        if (nsc.instance == nullptr && nsc.instantiate != nullptr) {
            nsc.instance = nsc.instantiate();
            nsc.instance->entity_ = Entity{handle, this};
            nsc.instance->on_create();
        }
    }

    for (auto [handle, nsc] : registry_.view<NativeScriptComponent>().each()) {
        if (nsc.enabled && nsc.instance != nullptr) {
            nsc.instance->on_update(dt);
        }
    }
}

void Scene::on_event(Event &event) {
    EventDispatcher dispatcher{event};

    // Handlers return false on purpose: scripts observe events, they do not
    // consume them.
    dispatcher.dispatch([this](KeyPressedEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_key_pressed(e.get_key_code(), e.is_repeat());
        });
        return false;
    });

    dispatcher.dispatch([this](KeyReleasedEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_key_released(e.get_key_code());
        });
        return false;
    });

    dispatcher.dispatch([this](MouseButtonPressedEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_mouse_button_pressed(e.get_mouse_button());
        });
        return false;
    });

    dispatcher.dispatch([this](MouseButtonReleasedEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_mouse_button_released(e.get_mouse_button());
        });
        return false;
    });

    dispatcher.dispatch([this](MouseScrolledEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_mouse_scrolled(e.get_x_offset(), e.get_y_offset());
        });
        return false;
    });

    dispatcher.dispatch([this](MouseMovedEvent &e) {
        for_each_enabled_script(registry_, [&e](ScriptableEntity &script) {
            script.on_mouse_moved(e.get_x(), e.get_y());
        });
        return false;
    });
}

void Scene::update_world_transforms() {
    for (Entity root : get_root_entities()) {
        update_world_recursive(root, glm::mat4{1.f}, 0);
    }
}

void Scene::update_world_recursive(Entity entity,
                                   const glm::mat4 &parent_world,
                                   int depth) {
    // Cycles are rejected by set_parent(), the depth cap only protects against
    // a corrupted RelationshipComponent.
    constexpr int kMaxDepth = 128;
    if (!entity || depth > kMaxDepth) {
        return;
    }

    glm::mat4 world = parent_world;
    if (auto *transform = entity.try_get_component<TransformComponent>()) {
        transform->world = parent_world * transform->load_matrix();
        world = transform->world;
    }

    const auto *relationship =
        entity.try_get_component<RelationshipComponent>();
    if (relationship == nullptr) {
        return;
    }
    for (auto child : relationship->children) {
        if (registry_.valid(child)) {
            update_world_recursive(Entity{child, this}, world, depth + 1);
        }
    }
}

void Scene::on_render() {
    update_world_transforms();

    glm::mat4 camera_projection{1.f};
    glm::mat4 camera_view{1.f};
    bool has_main_camera = false;

    for (auto [handle, camera, transform] :
         registry_.view<CameraComponent, TransformComponent>().each()) {
        if (camera.primary) {
            camera_projection = camera.camera.get_proj_matrix();
            // The camera entity stores its pose as a world transform, the view
            // matrix is its inverse.
            camera_view = glm::inverse(transform.world);
            has_main_camera = true;
            break;
        }
    }

    if (has_main_camera) {
        Renderer2D::begin_scene(camera_projection, camera_view);
        for (auto [handle, transform, sprite] :
             registry_.view<TransformComponent, SpriteRendererComponent>()
                 .each()) {
            // The quad is unit sized, the sprite's own size is applied here so
            // that a parent's TransformComponent::scale does not scale it.
            const glm::mat4 model =
                transform.world *
                glm::scale(glm::mat4{1.f}, glm::vec3{sprite.size, 1.f});
            if (sprite.sub_texture) {
                Renderer2D::draw_quad(model,
                                      sprite.sub_texture,
                                      sprite.tiling_factor,
                                      sprite.color);
            } else if (sprite.texture) {
                Renderer2D::draw_quad(
                    model, sprite.texture, sprite.tiling_factor, sprite.color);
            } else {
                Renderer2D::draw_quad(model, sprite.color);
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
