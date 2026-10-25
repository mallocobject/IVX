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

// Builds the script module target and reloads it. `log` receives the compiler
// output so the console can show errors. Blocks while compiling.
bool rebuild_scripts(ScriptLibrary &library, std::string &log);

} // namespace ivx
