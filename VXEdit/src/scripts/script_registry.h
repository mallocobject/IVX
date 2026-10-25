#pragma once

#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h"

#include <string>
#include <vector>

namespace ivx {

// Name <-> script type table.
//
// Scripts usually come from a module DLL (VXScripts), so this is filled at load
// time and cleared before the module is unloaded -- the function pointers would
// dangle afterwards. Scripts compiled into the editor can still register through
// the add<T>() helper.
//
// The function pointers identify the type: bind<T>() produces the same
// instantiate/destroy addresses for every call, so they work as a type key.
struct ScriptEntry {
    std::string name;
    ScriptableEntity *(*instantiate)(){nullptr};
    void (*destroy)(NativeScriptComponent *){nullptr};
};

class ScriptRegistry {
  public:
    static ScriptRegistry &get();

    template <typename T>
    void add(std::string name) {
        NativeScriptComponent probe; // stateless: only harvests the pointers
        probe.bind<T>();
        add_raw(std::move(name), probe.instantiate, probe.destroy);
    }

    // Raw registration, used by the module loader.
    void add_raw(std::string name,
                 ScriptableEntity *(*instantiate)(),
                 void (*destroy)(NativeScriptComponent *));

    void clear() {
        entries_.clear();
    }

    const std::vector<ScriptEntry> &entries() const {
        return entries_;
    }

    // Name of the type behind `instantiate`, or nullptr when unknown.
    const char *name_of(ScriptableEntity *(*instantiate)()) const;

    // Replaces the entity's script with `name`, remembering the name so the
    // binding survives a module reload. Returns false when the name is unknown.
    bool bind_by_name(const std::string &name, Entity entity) const;

  private:
    std::vector<ScriptEntry> entries_;
};

} // namespace ivx
