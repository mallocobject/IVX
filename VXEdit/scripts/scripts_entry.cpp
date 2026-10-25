// Entry point of the VXScripts module.
//
// Every script module exports the same C function; the host loads the DLL and
// calls it with its registrar (to publish script types) and its api table (so
// scripts can log / poll input without linking the engine).

#include "camera_controller.h"
#include "ivx/script/script_module.h"

#include <cstring>

using namespace ivx;

namespace {

template <typename T>
ScriptableEntity *create_script() {
    return new T();
}

template <typename T>
void destroy_script(NativeScriptComponent *component) {
    if (component->instance != nullptr) {
        delete static_cast<T *>(component->instance);
        component->instance = nullptr;
    }
}

} // namespace

extern "C" __declspec(dllexport) void
ivx_register_scripts(const ScriptRegistrar *registrar, const ScriptApi *api) {
    install_script_api(api);

    if (registrar == nullptr || registrar->version != kScriptModuleVersion ||
        registrar->add == nullptr) {
        return;
    }

    registrar->add("CameraController",
                   &create_script<CameraController>,
                   &destroy_script<CameraController>);
}
