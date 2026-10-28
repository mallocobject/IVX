#pragma once

#include "ivx/scene/scene.h"
#include <filesystem>

namespace ivx {
// Reads and writes a whole Scene as YAML: every entity with its Tag, Transform,
// Relationship, Camera, Sprite Renderer and Native Script components.
//
// Two things deliberately do not round-trip:
//
//   * GPU resources. A sprite's texture is a device handle; nothing about it
//   belongs in a
//     file, and the engine has no asset database yet. Sprites come back as flat
//     colour quads with the same size / tiling / tint (the file records
//     `HasTexture` so the reader can say so instead of silently dropping data).
//   * Script types. The name -> type table belongs to the host (ScriptRegistry
//   lives in
//     the editor), so a Native Script is stored by name and enabled flag. The
//     component comes back with the name but no instance; whoever owns the
//     scene binds it afterwards (see ScriptHost::bind_scene_scripts in the
//     editor).
//
// Values are written exactly as the engine holds them:
// TransformComponent::rotation is stored in radians because that is what
// build_rotation() consumes.
class Serializer {
  public:
    // Returns an empty Ref on failure (missing file, bad YAML, no Entities
    // sequence) and logs why.
    static Ref<Scene> deserialize(const std::filesystem::path &filepath);

    // Creates the parent directory. Logs instead of throwing when the write
    // fails.
    static void serialize(const Ref<Scene> &scene,
                          const std::filesystem::path &filepath);

  private:
    // Bumped when the on-disk layout changes in a way older readers cannot
    // handle.
    static constexpr int k_format_version = 1;
};
} // namespace ivx
