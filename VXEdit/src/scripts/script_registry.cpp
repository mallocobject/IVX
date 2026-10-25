#include "script_registry.h"

#include "ivx/core/logger.h"
#include "ivx/scene/components.h"

namespace ivx {

ScriptRegistry &ScriptRegistry::get() {
    static ScriptRegistry instance;
    return instance;
}

void ScriptRegistry::add_raw(std::string name,
                             ScriptableEntity *(*instantiate)(),
                             void (*destroy)(NativeScriptComponent *)) {
    ScriptEntry entry;
    entry.name = std::move(name);
    entry.instantiate = instantiate;
    entry.destroy = destroy;
    entries_.push_back(std::move(entry));
}

const char *ScriptRegistry::name_of(ScriptableEntity *(*instantiate)()) const {
    if (instantiate == nullptr) {
        return nullptr;
    }
    for (const ScriptEntry &entry : entries_) {
        if (entry.instantiate == instantiate) {
            return entry.name.c_str();
        }
    }
    return nullptr;
}

bool ScriptRegistry::bind_by_name(const std::string &name,
                                  Entity entity) const {
    if (!entity) {
        return false;
    }

    const ScriptEntry *found = nullptr;
    for (const ScriptEntry &entry : entries_) {
        if (entry.name == name) {
            found = &entry;
            break;
        }
    }
    if (found == nullptr) {
        IVX_ERROR("Unknown script '{}'", name);
        return false;
    }

    auto *component = entity.try_get_component<NativeScriptComponent>();
    if (component == nullptr) {
        return false;
    }

    // Replacing a script must not leak the old instance.
    if (component->instance != nullptr && component->destroy != nullptr) {
        component->instance->on_destroy();
        component->destroy(component);
    }

    component->instantiate = found->instantiate;
    component->destroy = found->destroy;
    component->enabled = true;
    // Remembered so the editor can re-bind after a module reload.
    component->script_name = found->name;

    const auto *tag = entity.try_get_component<TagComponent>();
    IVX_INFO("Bound script '{}' to '{}'",
             found->name,
             tag != nullptr ? tag->tag : "<unnamed>");
    return true;
}

} // namespace ivx
