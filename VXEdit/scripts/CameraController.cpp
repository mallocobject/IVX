#include "CameraController.h"

#include "ivx/script/script_module.h"
#include "ivx/scene/components.h"

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
    log("CameraController detached from '" +
        get_component<TagComponent>().tag + "'");
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
    // Wheel zooms by scaling the orthographic size. The setters are inline, so
    // this works from inside the module without linking the engine.
    auto &camera = get_component<CameraComponent>();
    if (camera.camera.get_projection_type() !=
        SceneCamera::ProjectionType::Orthographic) {
        return;
    }

    const float size = camera.camera.get_orthographic_size();
    const float factor = y_offset > 0.f ? 0.9f : 1.1f;
    const float next = glm::clamp(size * factor, 20.f, 20000.f);
    camera.camera.set_orthographic_size(next);
    log("CameraController: zoom -> " + std::to_string(next));
}
// Stays inside namespace ivx: IVX_REGISTER_SCRIPT pastes the type name into an
// identifier, so the type has to be unqualified.
IVX_REGISTER_SCRIPT(CameraController)
} // namespace ivx












