#pragma once

#include "ivx/core/key_codes.h"
#include "ivx/core/mouse_codes.h"
#include "ivx/scene/entity.h"
#include "ivx/script/script_module.h"

#include <string>

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

    // ---- event hooks ----
    // Push-based input, for things that polling handles badly (taps, toggles,
    // scroll). Parameters are plain values so nothing but the vtable crosses the
    // module boundary. Polling in on_update() stays available for continuous
    // input such as movement.
    virtual void on_key_pressed(KeyCode /*key*/, bool /*repeat*/) {
    }
    virtual void on_key_released(KeyCode /*key*/) {
    }
    virtual void on_mouse_button_pressed(MouseCode /*button*/) {
    }
    virtual void on_mouse_button_released(MouseCode /*button*/) {
    }
    virtual void on_mouse_scrolled(float /*x_offset*/, float /*y_offset*/) {
    }
    virtual void on_mouse_moved(float /*x*/, float /*y*/) {
    }

  protected:
    // Header-only conveniences: scripts compiled into a module DLL can not call
    // engine functions directly, these go through the host's api table.
    bool key_pressed(KeyCode key) const {
        return script_key_pressed(static_cast<int>(key));
    }
    bool mouse_pressed(MouseCode button) const {
        return script_mouse_pressed(static_cast<int>(button));
    }
    void log(const std::string &message) const {
        script_log(ScriptLogLevel::Info, message.c_str());
    }

    Entity entity_;
    friend class Scene;
};

struct NativeScriptComponent {
    ScriptableEntity *instance{nullptr};
    ScriptableEntity *(*instantiate)(){nullptr};
    void (*destroy)(NativeScriptComponent *){nullptr};
    bool enabled{false};
    // Name the type was bound through. Survives a module reload, which is what
    // lets the editor re-bind every script after the DLL was rebuilt.
    std::string script_name;

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
        script_name.clear();
    }
};
} // namespace ivx
