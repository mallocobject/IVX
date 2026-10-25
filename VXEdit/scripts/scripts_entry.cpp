// Entry point of the VXScripts module.
//
// Every script module exports the same C function; the host loads the DLL and
// calls it with its registrar (to publish script types) and its api table (so
// scripts can log / poll input without linking the engine).
//
// Individual scripts do not need to be listed here: they self register through
// IVX_REGISTER_SCRIPT(Type) at the bottom of their .cpp.

#include "ivx/script/script_module.h"

using namespace ivx;

extern "C" __declspec(dllexport) void
ivx_register_scripts(const ScriptRegistrar *registrar, const ScriptApi *api) {
    install_script_api(api);

    if (registrar == nullptr || registrar->version != kScriptModuleVersion) {
        return;
    }

    ScriptModuleBuilder::get().publish(registrar);
}
