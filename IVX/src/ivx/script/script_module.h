#pragma once

#include <cstdint>

namespace ivx {

// ---- script module boundary ----
//
// Script modules are separate DLLs that include the engine *headers* only: they
// never link the engine. Everything that is not header-only (logging, input) has
// to come back through this flat function table, which the host installs when it
// loads the module. Keeping it C-compatible means no STL/CRT coupling across the
// module boundary, which is what makes hot reload survivable.

inline constexpr std::uint32_t kScriptModuleVersion = 1;

class ScriptableEntity;
struct NativeScriptComponent;

struct ScriptApi {
    std::uint32_t version;
    void (*log)(int level, const char *message);
    bool (*key_pressed)(int key_code);
    bool (*mouse_pressed)(int button);
    void (*mouse_pos)(float *x, float *y);
};

// What a module uses to publish its script types to the host.
struct ScriptRegistrar {
    std::uint32_t version;
    void (*add)(const char *name,
                ScriptableEntity *(*instantiate)(),
                void (*destroy)(NativeScriptComponent *));
};

// The exported entry point every script module must provide:
//
//   extern "C" IVX_SCRIPT_MODULE_EXPORT
//   void ivx_register_scripts(const ivx::ScriptRegistrar *registrar,
//                             const ivx::ScriptApi *api);
//
// Each module gets its own copy of the slot below (inline variables are not
// merged across Windows DLLs), so the host's install and the module's install do
// not interfere.
inline const ScriptApi *g_script_api = nullptr;

inline void install_script_api(const ScriptApi *api) {
    g_script_api = api;
}

enum class ScriptLogLevel : int { Info = 0, Warn = 1, Error = 2 };

inline void script_log(ScriptLogLevel level, const char *message) {
    if (g_script_api != nullptr && g_script_api->log != nullptr) {
        g_script_api->log(static_cast<int>(level), message);
    }
}

inline bool script_key_pressed(int key_code) {
    return g_script_api != nullptr && g_script_api->key_pressed != nullptr &&
           g_script_api->key_pressed(key_code);
}

inline bool script_mouse_pressed(int button) {
    return g_script_api != nullptr && g_script_api->mouse_pressed != nullptr &&
           g_script_api->mouse_pressed(button);
}

inline void script_mouse_pos(float *x, float *y) {
    if (g_script_api != nullptr && g_script_api->mouse_pos != nullptr) {
        g_script_api->mouse_pos(x, y);
    } else {
        *x = 0.f;
        *y = 0.f;
    }
}

} // namespace ivx
