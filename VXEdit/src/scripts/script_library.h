#pragma once

#include <filesystem>
#include <string>

namespace ivx {
class Scene;

// Host side of the script module system: loads VXScripts.dll, hands it the
// registrar + api table, and can drop it again for a hot reload.
class ScriptLibrary {
  public:
    ~ScriptLibrary();

    // Registers every script the module exports into ScriptRegistry.
    bool load(const std::filesystem::path &dll, std::string &error);
    void unload();

    bool is_loaded() const {
        return module_ != nullptr;
    }
    const std::filesystem::path &path() const {
        return path_;
    }

    // Unloads and loads again, then re-binds every script whose name is still
    // present. Callers should unbind before, see EditLayer::reload_scripts().
    bool reload(std::string &error);

  private:
    void *module_{nullptr};
    std::filesystem::path path_;
};

// Builds only the script module (VXScripts.dll): the engine and the editor are
// separate targets and are not touched. `reconfigure` runs CMake first, which
// is only needed when the *set* of script files changed; plain source edits go
// straight to the compiler. `clean_first` discards the objects too, which is
// what a rejected module needs (stale objects would rebuild the same bad DLL).
// Blocks while compiling, so call it from a worker thread (see ScriptHost).
bool build_script_module(bool reconfigure, bool clean_first, std::string &log);

// True when the Ninja tree under SCRIPTS_BUILD_DIR cannot build this module as
// it is: no build.ninja yet, or a cache that remembers another configuration or
// another checkout path. A tree configured with a wrong root has include paths
// that do not exist and fails on the first #include, so the caller has to wipe
// it instead of trusting it.
bool script_project_needs_reconfigure();

// Captures the MSVC environment (vcvars64.bat) on a background thread, so the
// first script build does not have to wait for it.
void warm_up_script_build_environment();

} // namespace ivx
