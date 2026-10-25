#pragma once

#include "ivx/scene/entity.h"

namespace ivx {
class ScriptableEntity {
  public:
    virtual ~ScriptableEntity() = default;

    template <typename T>
    T &get_component() {
        return entity_.get_component<T>();
    }
    template <typename T>
    bool has_component() {
        return entity_.has_component<T>();
    }

    virtual void on_create() {
    }
    virtual void on_update(float /*dt*/) {
    }
    virtual void on_destroy() {
    }

  protected:
    Entity entity_;
    friend class Scene;
};

struct NativeScriptComponent {
    ScriptableEntity *instance{nullptr};
    ScriptableEntity *(*instantiate)(){nullptr};
    void (*destroy)(NativeScriptComponent *){nullptr};
    bool enabled{false};

    // An unbound component is valid: the editor creates it first and the user
    // picks a script afterwards. Scene::on_update() skips unbound entries.
    bool is_bound() const {
        return instantiate != nullptr;
    }

    template <typename T>
    void bind() {
        instantiate = []() -> ScriptableEntity * { return new T(); };
        destroy = [](NativeScriptComponent *nsc) {
            delete nsc->instance;
            nsc->instance = nullptr;
        };
    }

    // Drops the binding together with the live instance.
    void unbind() {
        if (instance != nullptr) {
            instance->on_destroy();
        }
        if (destroy != nullptr) {
            destroy(this);
        }
        instantiate = nullptr;
        destroy = nullptr;
        enabled = false;
    }
};
} // namespace ivx