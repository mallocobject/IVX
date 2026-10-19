#pragma once

#include "ivx/core/base.h"
#include "ivx/core/key_codes.h"
#include "ivx/core/mouse_codes.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <utility>

namespace ivx {

class Input : public RefCounted {
  public:
    virtual ~Input() = default;

    static bool is_key_pressed(KeyCode key) {
        return instance_->is_key_pressed_impl(key);
    }

    static bool IsMouseButtonPressed(MouseCode button) {
        return instance_->is_mouse_button_pressed_impl(button);
    }
    static std::pair<float, float> get_mouse_pos() {
        return instance_->get_mouse_pos_impl();
    }

    static Ref<Input> create();

  protected:
    virtual bool is_key_pressed_impl(KeyCode key) = 0;
    virtual bool is_mouse_button_pressed_impl(MouseCode button) = 0;
    virtual std::pair<float, float> get_mouse_pos_impl() = 0;

  private:
    static Input *instance_;
};

} // namespace ivx
