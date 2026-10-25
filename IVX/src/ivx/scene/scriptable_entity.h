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

    template <typename T>
    void bind() {
        instantiate = []() -> ScriptableEntity * { return new T(); };
        destroy = [](NativeScriptComponent *nsc) {
            delete nsc->instance;
            nsc->instance = nullptr;
        };
    }
};
} // namespace ivx