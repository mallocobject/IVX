#pragma once

#include "ivx/core/base.h"
#include "ivx/core/key_codes.h"
#include "ivx/core/mouse_codes.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <glm/glm.hpp>
#include <utility>

namespace ivx {

class Input : public RefCounted {
  public:
    // Global gate, e.g. disabled while the editor viewport is not focused so
    // typing in an inspector field does not move the camera. Scripts keep their
    // own `enabled` flag; this only silences polling.
    static void set_enabled(bool enabled);
    static bool is_enabled();

    static bool is_key_pressed(KeyCode key);
    static bool is_mouse_button_pressed(MouseCode button);
    static glm::vec2 get_mouse_pos();
};

} // namespace ivx
