#pragma once

#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h"

#include <string>
#include <vector>

namespace ivx {

// Name <-> script type table.
//
// Scripts are still compiled into the application, but the *binding* is now a
// runtime choice: the editor lists these names in the inspector, so a new script
// only needs one registration instead of a hand-written bind< T >() call.
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

        ScriptEntry entry;
        entry.name = std::move(name);
        entry.instantiate = probe.instantiate;
        entry.destroy = probe.destroy;
        entries_.push_back(std::move(entry));
    }

    const std::vector<ScriptEntry> &entries() const {
        return entries_;
    }

    // Name of the type behind `instantiate`, or nullptr when unknown.
    const char *name_of(ScriptableEntity *(*instantiate)()) const;

    // Replaces the entity's script with `name`. The old instance is destroyed
    // first. Returns false when the name is unknown.
    bool bind_by_name(const std::string &name, Entity entity) const;

  private:
    std::vector<ScriptEntry> entries_;
};

// Registers every script type the editor can bind (idempotent).
void register_all_scripts();

} // namespace ivx
