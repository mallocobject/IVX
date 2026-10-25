#include "script_library.h"

#include "ivx/core/input.h"
#include "ivx/core/logger.h"
#include "ivx/script/script_module.h"
#include "script_registry.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <vector>

#if defined(IVX_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace ivx {
namespace {

// ---- the api table handed to every script module ----
void api_log(int level, const char *message) {
    switch (static_cast<ScriptLogLevel>(level)) {
    case ScriptLogLevel::Warn:
        IVX_WARN("{}", message != nullptr ? message : "");
        return;
    case ScriptLogLevel::Error:
        IVX_ERROR("{}", message != nullptr ? message : "");
        return;
    case ScriptLogLevel::Info:
    default:
        IVX_INFO("{}", message != nullptr ? message : "");
        return;
    }
}

bool api_key_pressed(int key_code) {
    return Input::is_key_pressed(static_cast<KeyCode>(key_code));
}

bool api_mouse_pressed(int button) {
    return Input::is_mouse_button_pressed(static_cast<MouseCode>(button));
}

void api_mouse_pos(float *x, float *y) {
    const glm::vec2 position = Input::get_mouse_pos();
    if (x != nullptr) {
        *x = position.x;
    }
    if (y != nullptr) {
        *y = position.y;
    }
}

const ScriptApi g_api{kScriptModuleVersion,
                      &api_log,
                      &api_key_pressed,
                      &api_mouse_pressed,
                      &api_mouse_pos};

void registrar_add(const char *name,
                   ScriptableEntity *(*instantiate)(),
                   void (*destroy)(NativeScriptComponent *)) {
    ScriptRegistry::get().add_raw(name, instantiate, destroy);
}

const ScriptRegistrar g_registrar{kScriptModuleVersion, &registrar_add};

using RegisterScriptsFn = void (*)(const ScriptRegistrar *, const ScriptApi *);

} // namespace

ScriptLibrary::~ScriptLibrary() {
    unload();
}

bool ScriptLibrary::load(const std::filesystem::path &dll, std::string &error) {
#if defined(IVX_PLATFORM_WINDOWS)
    unload();

    HMODULE module = ::LoadLibraryW(dll.wstring().c_str());
    if (module == nullptr) {
        error = "LoadLibrary failed (" + std::to_string(::GetLastError()) +
                "): " + dll.string();
        return false;
    }

    auto *entry = reinterpret_cast<RegisterScriptsFn>(
        ::GetProcAddress(module, "ivx_register_scripts"));
    if (entry == nullptr) {
        ::FreeLibrary(module);
        error = "VXScripts.dll does not export ivx_register_scripts";
        return false;
    }

    module_ = module;
    path_ = dll;

    // The host also installs the table for itself, so code compiled into the
    // editor (and ScriptableEntity helpers used there) behaves identically.
    install_script_api(&g_api);
    entry(&g_registrar, &g_api);

    IVX_INFO("Script module loaded: {} ({} script(s))",
             dll.filename().string(),
             ScriptRegistry::get().entries().size());
    return true;
#else
    (void)dll;
    error = "script modules are only implemented for Windows";
    return false;
#endif
}

void ScriptLibrary::unload() {
#if defined(IVX_PLATFORM_WINDOWS)
    if (module_ == nullptr) {
        return;
    }

    // Entries must not outlive the module: their function pointers die with it.
    ScriptRegistry::get().clear();

    ::FreeLibrary(static_cast<HMODULE>(module_));
    module_ = nullptr;
    IVX_INFO("Script module unloaded: {}", path_.filename().string());
#endif
}

bool ScriptLibrary::reload(std::string &error) {
    const std::filesystem::path dll = path_;
    if (dll.empty()) {
        error = "no script module loaded yet";
        return false;
    }

    unload();
    return load(dll, error);
}

bool rebuild_scripts(ScriptLibrary &library, std::string &log) {
#if defined(IVX_PLATFORM_WINDOWS)
    // The caller must have unloaded the module first: the linker cannot overwrite
    // a DLL that is still mapped into the process.
    const std::filesystem::path dll = library.path();

    // MSBuild only relinks what changed, so this stays a sub-second operation for
    // a single edited script.
    const std::string command = std::string{"cmake --build \""} +
                                IVX_BUILD_DIR + "\" --config " +
                                IVX_BUILD_CONFIG + " --target VXScripts 2>&1";

    std::array<char, 512> buffer{};
    FILE *pipe = _popen(command.c_str(), "r");
    if (pipe == nullptr) {
        log = "failed to start the compiler";
        return false;
    }
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) !=
           nullptr) {
        log += buffer.data();
    }
    const int exit_code = _pclose(pipe);

    // Load even when the build failed: the linker did not touch the old DLL in
    // that case, so the previous code keeps working instead of leaving the editor
    // without any script types.
    std::string error;
    const bool loaded = library.load(dll, error);
    if (!loaded) {
        log += "\n" + error;
    }
    return exit_code == 0 && loaded;
#else
    (void)library;
    log = "script modules are only implemented for Windows";
    return false;
#endif
}

} // namespace ivx
