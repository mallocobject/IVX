#include "player_controller.h"

#include "ivx/core/input.h"
#include "ivx/core/key_codes.h"
#include "ivx/core/logger.h"
#include "ivx/scene/components.h"
#include <glm/glm.hpp>

namespace ivx {

void PlayerController::on_create() {
    IVX_CORE_INFO("PlayerController attached to '{}'", get_component<
                                                           TagComponent>()
                                                            .tag);
}

void PlayerController::on_update(float delta_time) {
    auto &transform = get_component<TransformComponent>();

    glm::vec2 movement{0.f};
    if (Input::is_key_pressed(IVX_KEY_W)) {
        movement.y += 1.f;
    }
    if (Input::is_key_pressed(IVX_KEY_S)) {
        movement.y -= 1.f;
    }
    if (Input::is_key_pressed(IVX_KEY_A)) {
        movement.x -= 1.f;
    }
    if (Input::is_key_pressed(IVX_KEY_D)) {
        movement.x += 1.f;
    }
    if (Input::is_key_pressed(IVX_KEY_Q)) {
        transform.rotation += turn_speed_ * delta_time;
    }
    if (Input::is_key_pressed(IVX_KEY_E)) {
        transform.rotation -= turn_speed_ * delta_time;
    }

    if (glm::dot(movement, movement) > 0.f) {
        transform.translation +=
            glm::vec3{glm::normalize(movement), 0.f} * move_speed_ * delta_time;
    }
}

} // namespace ivx
