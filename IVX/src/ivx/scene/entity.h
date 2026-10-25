#pragma once

#include "ivx/scene/components.h"
#include "ivx/scene/scene.h"
#include <cstdint>
#include <entt.hpp>
#include <string>
#include <vector>

namespace ivx {
class Scene;

class Entity {
  public:
    Entity() = default;
    Entity(entt::entity handle, Scene *scene) : handle_(handle), scene_(scene) {
    }

    template <typename T, typename... Args>
    T &add_component(Args &&...args) {
        return scene_->registry().emplace<T>(handle_,
                                             std::forward<Args>(args)...);
    }

    template <typename T>
    T &get_component() {
        return scene_->registry().get<T>(handle_);
    }

    template <typename T>
    const T &get_component() const {
        return scene_->registry().get<T>(handle_);
    }

    template <typename T>
    bool has_component() const {
        return scene_->registry().all_of<T>(handle_);
    }

    template <typename T>
    T *try_get_component() {
        return scene_->registry().try_get<T>(handle_);
    }

    template <typename T>
    const T *try_get_component() const {
        return scene_->registry().try_get<T>(handle_);
    }

    template <typename T>
    void remove_component() {
        scene_->registry().remove<T>(handle_);
    }

    const std::string &get_name() const { // TagComponent assistant
        return get_component<TagComponent>().tag;
    }

    // ---- hierarchy (RelationshipComponent) ----
    bool has_parent() const {
        const auto *rel = try_get_component<RelationshipComponent>();
        return rel != nullptr && rel->parent != entt::null;
    }
    Entity parent() const {
        const auto *rel = try_get_component<RelationshipComponent>();
        if (rel == nullptr || rel->parent == entt::null) {
            return {};
        }
        return Entity{rel->parent, scene_};
    }
    // Copy: the caller may destroy/reparent entities while iterating the
    // result.
    std::vector<entt::entity> children() const {
        const auto *rel = try_get_component<RelationshipComponent>();
        return rel != nullptr ? rel->children : std::vector<entt::entity>{};
    }
    bool is_root() const {
        return !has_parent();
    }

    entt::entity handle() const {
        return handle_;
    }
    Scene *scene() const {
        return scene_;
    }

    operator entt::entity() const {
        return handle_;
    }

    operator std::uintptr_t() const {
        return static_cast<std::uintptr_t>(handle_);
    }

    // True when the handle refers to a live entity. Unlike is_set(), this also
    // catches handles left behind by Scene::destroy_entity(), which is the
    // common way an editor panel ends up with a dangling entity.
    bool valid() const {
        return handle_ != entt::null && scene_ != nullptr &&
               scene_->registry().valid(handle_);
    }
    // Only checks that the handle was assigned, without touching the registry.
    bool is_set() const {
        return handle_ != entt::null && scene_ != nullptr;
    }

    explicit operator bool() const {
        return valid();
    }

    bool operator==(const Entity &other) const {
        return handle_ == other.handle_ && scene_ == other.scene_;
    }

  private:
    entt::entity handle_{entt::null};
    Scene *scene_{nullptr};
};
} // namespace ivx
