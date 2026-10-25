#include "camera_controller.h"
#include "ivx/core/logger.h"

namespace ivx {
void CameraController::on_create() {
    IVX_INFO("CameraController attached to '{}'",
             get_component<TagComponent>().tag);
}

void CameraController::on_update(float dt) {
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

    auto &tf = get_component<TransformComponent>();
    tf.translation += translation;
    tf.rotation += rotation;
}

void CameraController::on_destroy() {
    IVX_INFO("CameraController detached from '{}'",
             get_component<TagComponent>().tag);
}
} // namespace ivx
