#include "content_browser_panel.h"

#include "asset/asset_manager.h"
#include "asset/asset_payload.h"
#include "ivx/core/logger.h"

#include <algorithm>
#include <cstdio>
#include <format>
#include <imgui.h>

namespace ivx {
namespace {
constexpr float k_tile_size = 72.f;
constexpr float k_tile_padding = 4.f;
constexpr ImU32 k_label_color = IM_COL32(220, 220, 220, 255);
constexpr ImU32 k_selected_background = IM_COL32(66, 110, 174, 255);
constexpr ImU32 k_hovered_background = IM_COL32(255, 255, 255, 24);

// One colour per asset type, so a folder / texture / scene / mesh is
// recognisable before the name is read.
ImU32 type_color(AssetType type) {
    switch (type) {
    case AssetType::kDirectory:
        return IM_COL32(122, 106, 60, 255);
    case AssetType::kTexture:
        return IM_COL32(70, 100, 130, 255);
    case AssetType::kScene:
        return IM_COL32(80, 120, 80, 255);
    case AssetType::kMesh:
        return IM_COL32(120, 80, 120, 255);
    case AssetType::kUnknown:
    default:
        return IM_COL32(70, 70, 70, 255);
    }
}
} // namespace

ContentBrowserPanel::ContentBrowserPanel() : Panel("Content Browser", true) {
    std::error_code ec;
    root_ =
        std::filesystem::weakly_canonical(std::filesystem::path{ASSETS}, ec);
    if (ec) {
        root_ = std::filesystem::path{ASSETS};
    }
    current_ = root_;
}

void ContentBrowserPanel::on_imgui() {
    draw_toolbar();
    ImGui::Separator();

    const AssetDirectory directory = AssetDirectory::scan(current_);
    if (!directory.error.empty()) {
        ImGui::TextDisabled("%s", directory.error.c_str());
        return;
    }
    draw_entries(directory);
}

void ContentBrowserPanel::navigate_to(const std::filesystem::path &directory) {
    std::error_code ec;
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(directory, ec);
    const std::filesystem::path target = ec ? directory : canonical;
    if (!is_inside(root_, target)) {
        return; // never leaves assets/
    }

    current_ = target;
    selected_.clear();
}

void ContentBrowserPanel::draw_toolbar() {
    const bool at_root = current_ == root_;

    ImGui::BeginDisabled(at_root);
    if (ImGui::Button("Up") && !at_root) {
        navigate_to(current_.parent_path());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::SmallButton("assets")) {
        navigate_to(root_);
    }

    std::error_code ec;
    const std::filesystem::path relative =
        std::filesystem::relative(current_, root_, ec);
    if (!ec) {
        std::filesystem::path walk = root_;
        for (const std::filesystem::path &part : relative) {
            if (part == ".") {
                continue;
            }
            walk /= part;

            const std::string label = part.string();
            ImGui::SameLine();
            ImGui::TextDisabled("/");
            ImGui::SameLine();
            ImGui::PushID(label.c_str());
            if (ImGui::SmallButton(label.c_str())) {
                navigate_to(walk);
            }
            ImGui::PopID();
        }
    }
}

void ContentBrowserPanel::draw_entries(const AssetDirectory &directory) {
    ImGui::BeginChild("##entries", ImVec2{0.f, 0.f});

    if (directory.entries.empty()) {
        ImGui::TextDisabled("Empty folder");
    }

    const float available = ImGui::GetContentRegionAvail().x;
    const float cell = k_tile_size + k_tile_padding * 2.f;
    const auto columns =
        static_cast<std::size_t>(std::max(1.f, available / cell));

    for (std::size_t i = 0; i < directory.entries.size(); ++i) {
        if (i % columns != 0) {
            ImGui::SameLine();
        }
        draw_tile(directory.entries[i], i);
    }

    if (directory.truncated) {
        ImGui::NewLine();
        ImGui::TextDisabled("Showing the first %zu entries",
                            AssetDirectory::k_max_entries);
    }

    ImGui::EndChild();
}

void ContentBrowserPanel::draw_tile(const AssetEntry &entry,
                                    std::size_t index) {
    ImGui::PushID(static_cast<int>(index));

    const float label_height = ImGui::GetTextLineHeight();
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(
        "##tile", ImVec2{k_tile_size, k_tile_size + label_height + 2.f});

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();
    const bool double_clicked =
        hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

    if (clicked) {
        selected_ = entry.path;
    }
    if (double_clicked && entry.is_directory) {
        navigate_to(entry.path);
    }

    ImDrawList *draw = ImGui::GetWindowDrawList();
    const ImVec2 preview_min{origin.x + k_tile_padding,
                             origin.y + k_tile_padding};
    const ImVec2 preview_max{origin.x + k_tile_size - k_tile_padding,
                             origin.y + k_tile_size - k_tile_padding};
    const ImVec2 label_pos{origin.x + k_tile_padding, preview_max.y + 4.f};

    const ImVec2 tile_max{origin.x + k_tile_size,
                          origin.y + k_tile_size + label_height + 2.f};
    if (const bool selected = selected_ == entry.path; selected || hovered) {
        draw->AddRectFilled(origin,
                            tile_max,
                            selected ? k_selected_background
                                     : k_hovered_background,
                            4.f);
    }

    const Ref<Texture2D> thumbnail =
        entry.type == AssetType::kTexture ? thumbnail_for(entry.path) : nullptr;
    if (thumbnail) {
        draw->AddImage(static_cast<ImTextureID>(thumbnail->get_id()),
                       preview_min,
                       preview_max);
    } else {
        draw->AddRectFilled(
            preview_min, preview_max, type_color(entry.type), 3.f);

        const char *label = asset_type_name(entry.type);
        const ImVec2 text_size = ImGui::CalcTextSize(label);
        draw->AddText(
            ImVec2{(preview_min.x + preview_max.x - text_size.x) * 0.5f,
                   (preview_min.y + preview_max.y - text_size.y) * 0.5f},
            k_label_color,
            label);
    }

    // Names are longer than a tile: clip instead of wrapping into the next row.
    draw->PushClipRect(
        label_pos,
        ImVec2{origin.x + k_tile_size, label_pos.y + label_height + 2.f},
        true);
    draw->AddText(label_pos, k_label_color, entry.display_name.c_str());
    draw->PopClipRect();

    // Textures are the only importable asset so far.
    if (entry.type == AssetType::kTexture && ImGui::BeginDragDropSource()) {
        AssetPayload payload;
        payload.type = entry.type;
        std::snprintf(payload.path,
                      sizeof(payload.path),
                      "%s",
                      entry.path.string().c_str());
        ImGui::SetDragDropPayload(
            k_asset_payload_type, &payload, sizeof(payload));
        ImGui::TextUnformatted(entry.display_name.c_str());
        ImGui::EndDragDropSource();
    }

    if (hovered) {
        ImGui::SetTooltip("%s\n%s\n%s",
                          entry.display_name.c_str(),
                          asset_type_name(entry.type),
                          entry.path.string().c_str());
    }

    ImGui::PopID();
}

Ref<Texture2D>
ContentBrowserPanel::thumbnail_for(const std::filesystem::path &path) {
    const std::string key = path.string();
    if (const auto it = thumbnails_.find(key); it != thumbnails_.end()) {
        return it->second;
    }
    if (failed_thumbnails_.contains(key)) {
        return nullptr;
    }

    Ref<Texture2D> texture = AssetManager::load_texture(path);
    if (!texture) {
        failed_thumbnails_.insert(key);
        return nullptr;
    }

    thumbnails_.emplace(key, texture);
    return texture;
}

} // namespace ivx
