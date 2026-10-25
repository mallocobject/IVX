#pragma once

#include "editor_context.h"
#include "ivx/scene/entity.h"

#include <functional>
#include <string>
#include <vector>

namespace ivx {

// Per-type editor policy.
struct ComponentFlags {
    bool removable{true};
    // Whether the type shows up in "Add Component". Singleton-ish components
    // (Camera) are created through their own panel instead: bolting a camera
    // onto a sprite entity is almost always a mistake.
    bool addable{true};
    // Whether the type gets its own section in the inspector. Tag is edited by
    // the inspector header instead, so it must not be drawn twice.
    bool draw_in_inspector{true};
};

// One row of the inspector: how to query/add/remove a component type and how to
// draw its widgets. Everything is type erased here so the inspector itself does
// not have to know about the component types.
struct ComponentEntry {
    std::string name;
    ComponentFlags flags;
    std::function<bool(Entity)> has;
    std::function<void(Entity)> add;
    std::function<void(Entity)> remove;
    std::function<void(Entity, EditorContext &)> draw;
};

class ComponentRegistry {
  public:
    static ComponentRegistry &get();

    // `custom_add` defaults to plain add_component<T>().
    template <typename T>
    void add(std::string name,
             std::function<void(Entity, T &, EditorContext &)> draw,
             ComponentFlags flags = {},
             std::function<void(Entity)> custom_add = {}) {
        ComponentEntry entry;
        entry.name = std::move(name);
        entry.flags = flags;
        entry.has = [](Entity entity) { return entity.has_component<T>(); };
        entry.remove = [](Entity entity) { entity.remove_component<T>(); };
        if (custom_add) {
            entry.add = std::move(custom_add);
        } else {
            // if constexpr keeps add_component<T>() out of the instantiation
            // for types without a default constructor (CameraComponent).
            entry.add = [](Entity entity) {
                if constexpr (std::is_default_constructible_v<T>) {
                    entity.add_component<T>();
                }
            };
        }
        if (draw) {
            entry.draw = [draw](Entity entity, EditorContext &context) {
                draw(entity, entity.get_component<T>(), context);
            };
        }
        entries_.push_back(std::move(entry));
    }

    const std::vector<ComponentEntry> &entries() const {
        return entries_;
    }

    // Lets other panels reuse a component's editor UI (CameraPanel does this
    // for CameraComponent).
    const ComponentEntry *find(std::string_view name) const {
        for (const auto &entry : entries_) {
            if (entry.name == name) {
                return &entry;
            }
        }
        return nullptr;
    }

    void clear() {
        entries_.clear();
    }

  private:
    std::vector<ComponentEntry> entries_;
};

// Registers every component type the editor knows about (idempotent).
void register_all_components();

} // namespace ivx
