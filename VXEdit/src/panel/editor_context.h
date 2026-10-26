#pragma once

#include "ivx/scene/entity.h"
#include "ivx/scene/scene.h"
#include <entt.hpp>

namespace ivx {

// Shared editor state. Panels never talk to each other directly: they read and
// write this context, which keeps selection and the scene in one place.
struct EditorContext {
    Ref<Scene> scene;
    entt::entity selected{entt::null};

    // ---- selection ----
    bool has_selection() const {
        return scene && selected != entt::null &&
               scene->registry().valid(selected);
    }
    void clear_selection() {
        selected = entt::null;
    }
    void select(Entity entity) {
        // selected = entity ? entity.handle() : entt::null;
        selected = entity.handle();
    }
    Entity selected_entity() const {
        if (!has_selection()) {
            return Entity{};
        }
        return Entity{selected, scene.get()};
    }
    // Drops the selection when the entity disappeared (deleted from another
    // panel, scene reload, ...). Call once per frame.
    void validate_selection() {
        if (selected != entt::null &&
            (!scene || !scene->registry().valid(selected))) {
            selected = entt::null;
        }
    }
};

} // namespace ivx
