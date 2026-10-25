#include "controller.h"
#include "ivx/core/logger.h"

namespace ivx {
void Controller::on_create() {
    IVX_CORE_DEBUG("Controller attached to '{}'",
                   get_component<TagComponent>().value);
}

void Controller::on_update(float dt) {
    static constexpr float speed = 500.f;
    static constexpr float delta = 180.f;
    glm::vec3 translation{0.f};
    float rotation = 0.f;

    glm::vec2 movement{0.f};
    if (Input::is_key_pressed(KeyCode::W)) {
        movement.y++;
    }
    if (Input::is_key_pressed(KeyCode::S)) {
        movement.y--;
    }
    if (Input::is_key_pressed(KeyCode::A)) {
        movement.x--;
    }
    if (Input::is_key_pressed(KeyCode::D)) {
        movement.x++;
    }

    if (movement.x != 0) {
        translation.x += glm::normalize(movement).x * speed * dt;
    }

    if (movement.y != 0) {
        translation.y += glm::normalize(movement).y * speed * dt;
    }

    if (Input::is_key_pressed(KeyCode::Q)) {
        rotation += delta * dt;
    }
    if (Input::is_key_pressed(KeyCode::E)) {
        rotation -= delta * dt;
    }

    auto &tf = get_component<TransformComponent>().transform;
    tf = glm::translate(tf, translation);
    tf = glm::rotate(tf, glm::radians(rotation), glm::vec3{0, 0, 1});
}

void Controller::on_destroy() {
}
} // namespace ivx