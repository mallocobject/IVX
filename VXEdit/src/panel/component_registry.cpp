#include "component_registry.h"

namespace ivx {

ComponentRegistry &ComponentRegistry::get() {
    static ComponentRegistry instance;
    return instance;
}

} // namespace ivx
