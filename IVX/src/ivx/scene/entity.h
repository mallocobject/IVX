#pragma once

#include "ivx/scene/components.h"
#include "ivx/scene/scene.h"
#include <entt.hpp>
#include <string>

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
    void remove_component() {
        scene_->registry().remove<T>(handle_);
    }

    const std::string &get_name() const { // TagComponent assistant
        return get_component<TagComponent>().value;
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
    explicit operator bool() const {
        return handle_ != entt::null && scene_ != nullptr;
    }

    bool operator==(const Entity &other) const {
        return handle_ == other.handle_ && scene_ == other.scene_;
    }

  private:
    entt::entity handle_{entt::null};
    Scene *scene_{nullptr};
};
} // namespace ivx
