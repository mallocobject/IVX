#pragma once

#include "script_library.h"

#include <filesystem>
#include <string>
#include <vector>

namespace ivx {
class Scene;

// Owns the script module and everything that mutates it: load at startup,
// generate new script sources from a template, rebuild + hot reload.
//
// Lives in the editor (not in the engine) because it deals with source files and
// the build system, and holds the scene pointer needed to re-bind scripts after a
// reload.
class ScriptHost {
  public:
    // Where generated scripts are written; passed in by CMake.
    ScriptHost();

    void set_scene(Scene *scene) {
        scene_ = scene;
    }

    bool load_initial(std::string &error);

    // Destroys every instance, rebuilds the module, loads it and re-binds the
    // scripts by name. `log` receives the compiler output.
    bool build_and_reload(std::string &log);

    // Writes scripts/<name>.h and .cpp from the template. Does not rebuild.
    bool create_script(const std::string &name,
                       std::filesystem::path &header_path,
                       std::filesystem::path &source_path,
                       std::string &error);

    std::filesystem::path script_path(const std::string &name) const;

    // Duration of the last build, for the panel's status line.
    float last_build_ms() const {
        return last_build_ms_;
    }
    bool last_build_reconfigured() const {
        return last_build_reconfigured_;
    }

    const std::filesystem::path &scripts_dir() const {
        return scripts_dir_;
    }
    bool is_loaded() const {
        return library_.is_loaded();
    }

  private:
    void unbind_all();
    // True when the set of files in scripts/ differs from the last build, i.e.
    // when CMake has to be re-run for the GLOB.
    bool scripts_dir_changed();

    ScriptLibrary library_;
    Scene *scene_{nullptr};
    std::filesystem::path scripts_dir_;
    std::vector<std::string> known_sources_;
    float last_build_ms_{0.f};
    bool last_build_reconfigured_{false};
    bool has_built_once_{false};
};

} // namespace ivx
