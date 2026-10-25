#include "rotator.h"
#include "ivx/core/logger.h"

namespace ivx {
void Rotator::on_create() {
    IVX_CORE_DEBUG("Rotator attached to '{}'",
                   get_component<TagComponent>().value);
}

void Rotator::on_update(float dt) {
    static constexpr float delta = 180.f;
    float rotation = 0.f;

    if (Input::is_key_pressed(IVX_KEY_Q)) {
        rotation += delta * dt;
    }
    if (Input::is_key_pressed(IVX_KEY_E)) {
        rotation -= delta * dt;
    }

    auto &tf = get_component<TransformComponent>().transform;
    tf = glm::rotate(tf, glm::radians(rotation), glm::vec3{0, 0, 1});
}

void Rotator::on_destroy() {
}
} // namespace ivx