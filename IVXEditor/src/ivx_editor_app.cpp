
#include <ivx.h>
#include <ivx/core/entry_point.h>

#include "editor_layer.h"

namespace ivx {
class IVXEditorApp : public Application {
  public:
    IVXEditorApp() : Application("IVX Editor") {
        push_layer(new EditorLayer);
    }

    ~IVXEditorApp() {
    }
};

Application *create_application() {
    return new IVXEditorApp;
}

} // namespace ivx