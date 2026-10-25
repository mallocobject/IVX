#pragma once

#include "script_library.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ivx {
class Scene;

// Owns the script module and everything that mutates it: load at startup,
// generate new script sources from a template, build + hot reload.
//
// The build runs on a worker thread: MSBuild takes seconds and doing that on
// the UI thread freezes the editor. The main thread follows it through
// poll_build(), which also performs the reload (loading a DLL and touching
// scene components has to stay on the main thread).
class ScriptHost {
  public:
    // Where generated scripts are written; passed in by CMake.
    ScriptHost();
    ~ScriptHost(); // joins a running build

    void set_scene(Scene *scene) {
        scene_ = scene;
    }

    bool load_initial(std::string &error);

    // Starts a build. Instances are unbound and the module unloaded first,
    // because the linker cannot overwrite a DLL that is still mapped in.
    // Returns false when a build is already running.
    bool begin_build();

    // Call once per frame. When the worker finished it joins it, loads the
    // fresh module and re-binds every script by name.
    void poll_build();

    bool is_building() const {
        return building_;
    }
    // Elapsed seconds while building, duration of the last build otherwise.
    float build_seconds() const {
        return build_seconds_;
    }
    bool last_build_ok() const {
        return last_build_ok_;
    }
    bool last_build_reconfigured() const {
        return last_build_reconfigured_;
    }
    const std::string &build_log() const {
        return last_log_;
    }

    // Writes scripts/<name>.h and .cpp from the template. Does not rebuild.
    bool create_script(const std::string &name,
                       std::filesystem::path &header_path,
                       std::filesystem::path &source_path,
                       std::string &error);

    std::filesystem::path script_path(const std::string &name) const;

    const std::filesystem::path &scripts_dir() const {
        return scripts_dir_;
    }
    bool is_loaded() const {
        return library_.is_loaded();
    }

  private:
    struct PendingBinding {
        std::uint32_t handle;
        std::string name;
    };

    void unbind_all();
    // True when the generated project file no longer matches the files in
    // scripts/, i.e. when CMake has to be re-run for the GLOB.
    bool needs_reconfigure() const;

    ScriptLibrary library_;
    Scene *scene_{nullptr};
    std::filesystem::path scripts_dir_;
    std::vector<PendingBinding> pending_bindings_;

    std::thread worker_;
    std::atomic<bool> worker_done_{false};
    // Result handoff from the worker thread.
    std::mutex result_mutex_;
    std::string worker_log_;
    bool worker_ok_{false};

    bool building_{false};
    bool last_build_ok_{false};
    bool last_build_reconfigured_{false};
    float build_seconds_{0.f};
    std::chrono::steady_clock::time_point build_start_{};
    std::string last_log_;
};

} // namespace ivx
