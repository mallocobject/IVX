#include "script_library.h"

#include "ivx/core/input.h"
#include "ivx/core/logger.h"
#include "ivx/script/script_abi.h"
#include "ivx/script/script_module.h"
#include "script_registry.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>
#include <thread>
#include <utility>
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
using ScriptAbiFn = const char *(*)();

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

    // Layout handshake first: a module built against a different engine layout
    // (component gained a member, vtable reordered, other CRT, other Ref
    // flavour) would read the wrong offsets and take the process down inside
    // the DLL. Refusing it turns a crash into a rebuild.
    auto *abi = reinterpret_cast<ScriptAbiFn>(
        ::GetProcAddress(module, "ivx_script_module_abi"));
    if (abi == nullptr) {
        ::FreeLibrary(module);
        error =
            dll.filename().string() +
            " has no ivx_script_module_abi export (built by an older editor?)";
        return false;
    }

    const std::string module_abi = abi() != nullptr ? std::string{abi()} : "?";
    const std::string editor_abi = script_abi_fingerprint();
    if (module_abi != editor_abi) {
        ::FreeLibrary(module);
        error = dll.filename().string() +
                " was built for a different engine (" + module_abi +
                ", editor is " + editor_abi + ")";
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

namespace {
// Runs a shell command and appends its combined stdout/stderr to `log`.
int run_command(const std::string &command, std::string &log) {
    std::array<char, 512> buffer{};
    FILE *pipe = _popen(command.c_str(), "r");
    if (pipe == nullptr) {
        log += "failed to start: " + command + "\n";
        return -1;
    }
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) !=
           nullptr) {
        log += buffer.data();
    }
    return _pclose(pipe);
}

// The Ninja build of the script module needs the MSVC environment (cl.exe,
// INCLUDE, LIB). vcvars64.bat takes a couple of seconds, so the environment is
// captured once in the background (warm_up_script_build_environment) and only
// *applied* on the build thread: _putenv_s is not thread safe, running vcvars
// is.
std::mutex g_env_mutex;
std::vector<std::pair<std::string, std::string>> g_env_values;
bool g_env_captured = false;
bool g_env_failed = false;
bool g_env_applied = false;

bool capture_msvc_environment(std::string &log) {
    const std::filesystem::path vcvars = VCVARS_PATH;
    if (vcvars.empty() || !std::filesystem::exists(vcvars)) {
        log += "vcvars64.bat not found (VCVARS_PATH); cannot set up MSVC\n";
        return false;
    }

    // `set` prints the environment as KEY=VALUE lines after vcvars ran. The
    // whole command is wrapped in quotes because it starts with one (the path
    // contains spaces) and _popen() hands it to `cmd /c`.
    std::string output;
    const std::string inner =
        "\"" + vcvars.string() + "\" >nul 2>&1 && set 2>&1";
    if (run_command("\"" + inner + "\"", output) != 0) {
        log += "failed to run vcvars64.bat\n" + output;
        return false;
    }

    std::size_t pos = 0;
    while (pos < output.size()) {
        const std::size_t end = output.find('\n', pos);
        std::string line = output.substr(
            pos, end == std::string::npos ? std::string::npos : end - pos);
        pos = end == std::string::npos ? output.size() : end + 1;
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        // Skip the "=C:=C:\..." style entries and anything without a name.
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos || equals == 0) {
            continue;
        }
        g_env_values.emplace_back(line.substr(0, equals),
                                  line.substr(equals + 1));
    }

    IVX_CORE_DEBUG("Scripts: captured MSVC environment ({} variables)",
                   g_env_values.size());
    return !g_env_values.empty();
}

bool ensure_msvc_environment(std::string &log) {
    std::lock_guard<std::mutex> lock{g_env_mutex};
    if (g_env_applied) {
        return true;
    }
    if (g_env_failed) {
        return false;
    }

    if (!g_env_captured) {
        g_env_failed = !capture_msvc_environment(log);
        g_env_captured = true;
        if (g_env_failed) {
            return false;
        }
    }

    for (const auto &[name, value] : g_env_values) {
        _putenv_s(name.c_str(), value.c_str());
    }
    g_env_applied = true;
    return true;
}

// Reads one cache entry out of an existing Ninja tree, so a stale configuration
// can be told apart from a matching one without running CMake.
std::string cached_entry(const std::filesystem::path &build_dir,
                         std::string_view key) {
    std::ifstream cache{build_dir / "CMakeCache.txt"};
    std::string line;
    while (std::getline(cache, line)) {
        if (line.rfind(key, 0) != 0) {
            continue;
        }
        const std::size_t equals = line.find('=');
        return equals == std::string::npos ? std::string{}
                                           : line.substr(equals + 1);
    }
    return {};
}

std::string cached_build_type(const std::filesystem::path &build_dir) {
    return cached_entry(build_dir, "CMAKE_BUILD_TYPE:");
}

// The repository root the tree was configured against. A tree configured with a
// relative or unexpanded root ($PWD, %CD%, a moved checkout, ...) has include
// paths that do not exist, so every build fails on the first #include until the
// tree is wiped - that is the "the DLL never links" case, and it has to heal
// itself.
std::string cached_ivx_root(const std::filesystem::path &build_dir) {
    return cached_entry(build_dir, "IVX_ROOT:");
}

// Configures the standalone Ninja project. Only needed once (or when the
// generator/build directory was removed): new or removed scripts are handled by
// CONFIGURE_DEPENDS, which makes Ninja re-run CMake on its own.
bool configure_script_project(std::string &log) {
    const std::filesystem::path build_dir = SCRIPTS_BUILD_DIR;

    const bool configured = std::filesystem::exists(build_dir / "build.ninja");
    if (configured) {
        // A Ninja tree caches CMAKE_BUILD_TYPE, so `-DCMAKE_BUILD_TYPE` is
        // ignored on a tree that was configured for the other configuration.
        // Mixing them is not a cosmetic mismatch: Debug and Release use
        // different CRTs and iterator debug levels, so std containers change
        // layout and the module corrupts whatever it is handed. Start over
        // instead.
        const std::string cached = cached_build_type(build_dir);
        const std::string root = cached_ivx_root(build_dir);

        const bool wrong_config = !cached.empty() && cached != IVX_BUILD_CONFIG;
        const bool wrong_root =
            !root.empty() && root != std::string{IVX_ROOT_DIR};

        if (!wrong_config && !wrong_root) {
            return true; // already configured for this configuration and
                         // checkout
        }

        if (wrong_config) {
            log += "script build directory was configured for " + cached +
                   ", rebuilding for " + IVX_BUILD_CONFIG + "\n";
        }
        if (wrong_root) {
            log += "script build directory points at '" + root +
                   "' instead of '" + std::string{IVX_ROOT_DIR} +
                   "', reconfiguring\n";
        }

        std::error_code ignored;
        std::filesystem::remove_all(build_dir, ignored);
    }

    std::string command = "cmake -S \"" + std::string{SCRIPTS_PROJECT_DIR} +
                          "\" -B \"" + build_dir.string() + "\" -G Ninja" +
                          " -DCMAKE_BUILD_TYPE=" + IVX_BUILD_CONFIG +
                          " -DIVX_ROOT=\"" + IVX_ROOT_DIR + "\" 2>&1";
    const int exit_code = run_command("\"" + command + "\"", log);
    if (exit_code != 0) {
        log += "\nCMake configure of the script project failed\n";
        return false;
    }
    return true;
}
} // namespace

bool script_project_needs_reconfigure() {
    const std::filesystem::path build_dir = SCRIPTS_BUILD_DIR;
    if (!std::filesystem::exists(build_dir / "build.ninja")) {
        return true;
    }

    const std::string cached = cached_build_type(build_dir);
    const std::string root = cached_ivx_root(build_dir);
    if (!cached.empty() && cached != IVX_BUILD_CONFIG) {
        return true;
    }
    return !root.empty() && root != std::string{IVX_ROOT_DIR};
}

void warm_up_script_build_environment() {
#if defined(IVX_PLATFORM_WINDOWS)
    // Runs vcvars64.bat ahead of time so the first build does not pay for it.
    std::thread{[] {
        std::string ignored;
        std::lock_guard<std::mutex> lock{g_env_mutex};
        if (!g_env_captured) {
            g_env_failed = !capture_msvc_environment(ignored);
            g_env_captured = true;
        }
    }}.detach();
#endif
}

bool build_script_module(bool reconfigure, std::string &log) {
#if defined(IVX_PLATFORM_WINDOWS)
    if (!ensure_msvc_environment(log)) {
        return false;
    }

    if (reconfigure && !configure_script_project(log)) {
        return false;
    }

    // `cmake --build` on the standalone project: Ninja only touches what
    // changed and re-runs CMake itself when the source glob changes. Nothing
    // here knows about the editor's solution, so a script rebuild cannot
    // invalidate it.
    const std::string build_command =
        "cmake --build \"" + std::string{SCRIPTS_BUILD_DIR} + "\" 2>&1";
    IVX_CORE_DEBUG("script build command: {}", build_command);
    const int build_exit = run_command("\"" + build_command + "\"", log);
    if (log.empty()) {
        log = build_command; // keep the log informative even on a no-op build
    }
    return build_exit == 0;
#else
    (void)reconfigure;
    log = "script modules are only implemented for Windows";
    return false;
#endif
}

} // namespace ivx
