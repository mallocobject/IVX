#include "Test.h"

#include "ivx/script/script_module.h"
#include "ivx/scene/components.h"

#include <string>

namespace ivx {

void Test::on_create() {
    log("Test attached to '" + get_component<TagComponent>().tag + "'");
}

void Test::on_update(float dt) {
    (void)dt;
    // auto &transform = get_component<TransformComponent>();
    // transform.rotation += 45.f * dt;
}

void Test::on_destroy() {
}

// Stays inside namespace ivx: IVX_REGISTER_SCRIPT pastes the type name into an
// identifier, so the type has to be unqualified.
IVX_REGISTER_SCRIPT(Test)

} // namespace ivx
