// Entry point of the VXScripts module.
//
// Every script module exports the same two C functions:
//
//   ivx_script_module_abi()  the layout fingerprint this module was compiled
//   against ivx_register_scripts()   called by the host with its registrar (to
//   publish script
//                            types) and its api table (so scripts can log /
//                            poll input without linking the engine)
//
// Individual scripts do not need to be listed here: they self register through
// IVX_REGISTER_SCRIPT(Type) at the bottom of their .cpp.

#include "ivx/script/script_abi.h"
#include "ivx/script/script_module.h"

using namespace ivx;

// Checked before anything else is touched: a module built against a different
// engine layout would otherwise read components at the wrong offsets.
extern "C" __declspec(dllexport) const char *ivx_script_module_abi() {
    static const std::string abi = script_abi_fingerprint();
    return abi.c_str();
}

extern "C" __declspec(dllexport) void
ivx_register_scripts(const ScriptRegistrar *registrar, const ScriptApi *api) {
    install_script_api(api);

    if (registrar == nullptr || registrar->version != kScriptModuleVersion) {
        return;
    }

    ScriptModuleBuilder::get().publish(registrar);
}
