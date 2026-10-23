
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
    return new VEXEdit;
}

} // namespace ivx