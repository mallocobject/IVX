
#include <ivx.h>
#include <ivx/core/ivx_main.h>

#include "edit_layer.h"

namespace ivx {
class VEXEdit : public Application {
  public:
    VEXEdit() : Application("VEXEdit") {
        push_layer(new EditLayer);
    }

    ~VEXEdit() {
    }
};

Application *create_application() {
    // Before the window/renderer exist: window creation, GL info and the first
    // scene messages should already land in the Console panel.
    enable_console_capture(true);
    return new VEXEdit;
}

} // namespace ivx