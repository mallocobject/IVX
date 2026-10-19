#pragma once

#include "ivx/core/input.h"

namespace ivx {

class WinInput : public Input {
  protected:
    virtual bool is_key_pressed_impl(KeyCode key) override;
    virtual bool is_mouse_button_pressed_impl(MouseCode button) override;
    virtual std::pair<float, float> get_mouse_pos_impl() override;
};

} // namespace ivx
