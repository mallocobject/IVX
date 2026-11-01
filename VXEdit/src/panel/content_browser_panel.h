#pragma once

#include "asset/asset_directory.h"
#include "ivx/render/texture.h"
#include "panel.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ivx {

// Browses VXEdit/assets and hands textures to other panels by drag and drop.
// Navigation is clamped to the asset root: there is no way out of it, not even
// with "..", because the breadcrumb only offers segments below the root.
class ContentBrowserPanel : public Panel {
  public:
    ContentBrowserPanel();

  protected:
    void on_imgui() override;

  private:
    void navigate_to(const std::filesystem::path &directory);
    void draw_toolbar();
    void draw_entries(const AssetDirectory &directory);
    void draw_tile(const AssetEntry &entry, std::size_t index);
    // Decoded once and kept for the panel's lifetime; a failed load is
    // remembered so a broken image is not re-read every frame.
    Ref<Texture2D> thumbnail_for(const std::filesystem::path &path);

    std::filesystem::path root_;
    std::filesystem::path current_;
    std::filesystem::path selected_;
    std::unordered_map<std::string, Ref<Texture2D>> thumbnails_;
    std::unordered_set<std::string> failed_thumbnails_;
};

} // namespace ivx
