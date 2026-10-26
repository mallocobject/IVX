#include "CameraController.h"

#include "ivx/scene/components.h"
#include "ivx/script/script_module.h"

#include <glm/glm.hpp>
#include <string>

namespace ivx {
void CameraController::on_create() {
    log("CameraController attached to '" + get_component<TagComponent>().tag +
        "'");
}

void CameraController::on_update(float dt) {
    static constexpr float speed = 500.f;
    static constexpr float delta = 180.f;

    glm::vec2 movement{0.f};
    if (key_pressed(KeyCode::W)) {
        movement.y++;
    }
    if (key_pressed(KeyCode::S)) {
        movement.y--;
    }
    if (key_pressed(KeyCode::A)) {
        movement.x--;
    }
    if (key_pressed(KeyCode::D)) {
        movement.x++;
    }

    glm::vec3 translation{0.f};
    if (movement.x != 0 || movement.y != 0) {
        const glm::vec2 direction = glm::normalize(movement);
        translation.x = direction.x * speed * dt;
        translation.y = direction.y * speed * dt;
    }

    float rotation = 0.f;
    if (key_pressed(KeyCode::Q)) {
        rotation += delta * dt;
    }
    if (key_pressed(KeyCode::E)) {
        rotation -= delta * dt;
    }

    auto &transform = get_component<TransformComponent>();
    transform.translation += translation;
    transform.rotation += rotation;
}

void CameraController::on_destroy() {
    log("CameraController detached from '" + get_component<TagComponent>().tag +
        "'");
}

void CameraController::on_key_pressed(KeyCode key, bool repeat) {
    // Discrete input: fires once per press, no edge detection needed.
    if (repeat) {
        return; // ignore auto-repeat
    }

    if (key == KeyCode::Space) {
        log("CameraController: Space pressed on '" +
            get_component<TagComponent>().tag + "'");
    }
    if (key == KeyCode::R) {
        auto &transform = get_component<TransformComponent>();
        transform.translation = glm::vec3{0.f, 0.f, transform.translation.z};
        transform.rotation = 0.f;
        log("CameraController: transform reset");
    }
}

void CameraController::on_mouse_scrolled(float /*x_offset*/, float y_offset) {
    if (y_offset == 0.f || !has_component<CameraComponent>()) {
        return;
    }

    // Offsets are not always +-1: a fast wheel sends one event with 2 or 3, and
    // precision touchpads send fractions. Scaling by the amount keeps "one
    // notch = one step" true in both cases.
    auto &camera = get_component<CameraComponent>();

    const float factor = glm::pow(0.9f, y_offset);

    // Orthographic: zoom by scaling the visible world height.
    if (camera.camera.get_projection_type() ==
        SceneCamera::ProjectionType::Orthographic) {
        const float size = camera.camera.get_orthographic_size();
        const float next = glm::clamp(size * factor, 20.f, 20000.f);
        camera.camera.set_orthographic_size(next);
        return;
    }

    // Perspective: zoom by narrowing the vertical field of view.
    //
    // The angle is in degrees (Unity/Unreal convention);
    // SceneCamera::get_proj_matrix() converts it to radians for
    // glm::perspective(). The limits are degrees too: 1..179.
    constexpr float kMinFov = 1.f;   // 1 degree
    constexpr float kMaxFov = 179.f; // 179 degrees
    const float fov = camera.camera.get_perspective_vertical_fov();
    const float next = glm::clamp(fov * factor, kMinFov, kMaxFov);
    camera.camera.set_perspective_vertical_fov(next);
}
// Stays inside namespace ivx: IVX_REGISTER_SCRIPT pastes the type name into an
// identifier, so the type has to be unqualified.
IVX_REGISTER_SCRIPT(CameraController)
} // namespace ivx
