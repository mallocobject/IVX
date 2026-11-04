#include "environment_panel.h"

#include "form_layout.h"
#include "ivx/scene/environment.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace ivx {

EnvironmentPanel::EnvironmentPanel(EditorContext &context)
    : Panel("Environment"), context_(context) {
    set_default_size(ImVec2{380.f, 240.f});
}

void EnvironmentPanel::on_imgui() {
    if (!context_.scene) {
        ImGui::TextDisabled("No scene loaded.");
        return;
    }

    Environment &environment = context_.scene->environment();

    ImGui::SeparatorText("Ambient light");

    label_row("Color", [&] {
        ImGui::ColorEdit3("##ambient",
                          glm::value_ptr(environment.ambient),
                          ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
    });
    ImGui::Spacing();
    ImGui::SeparatorText("Tone mapping");

    int curve = static_cast<int>(environment.tonemap);
    label_row("Curve", [&] {
        if (ImGui::Combo("##tonemap_curve",
                         &curve,
                         "Reinhard\0ACES\0Khronos Neutral\0")) {
            environment.tonemap = static_cast<ToneMappingCurve>(curve);
        }
    });
    label_row("Exposure", [&] {
        ImGui::SliderFloat("##exposure", &environment.exposure, 0.1f, 4.f);
    });

    ImGui::Spacing();
    ImGui::SeparatorText("Shadows");
    int resolution = 2;
    for (int i = 0; i < static_cast<int>(k_shadow_resolutions.size()); ++i) {
        if (k_shadow_resolutions[i] == environment.shadows.resolution)
            resolution = i;
    }
    label_row("Resolution", [&] {
        if (ImGui::Combo("##shadow_resolution",
                         &resolution,
                         "512\0"
                         "1024\0"
                         "2048\0"
                         "4096\0")) {
            environment.shadows.resolution = k_shadow_resolutions[resolution];
        }
    });
    label_row("Distance", [&] {
        ImGui::DragFloat("##shadow_distance",
                         &environment.shadows.distance,
                         1.f,
                         0.1f,
                         10000.f,
                         "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
    });

    ImGui::Spacing();
    ImGui::SeparatorText("Ground");
    auto &ground = environment.ground;
    label_row("Enabled",
              [&] { ImGui::Checkbox("##ground_enabled", &ground.enabled); });
    if (ground.enabled) {
        label_row("Height", [&] {
            ImGui::DragFloat("##ground_height", &ground.height, 0.1f);
        });
        label_row("Grid spacing", [&] {
            ImGui::DragFloat("##ground_spacing",
                             &ground.grid_spacing,
                             0.1f,
                             0.01f,
                             10000.f,
                             "%.2f",
                             ImGuiSliderFlags_AlwaysClamp);
        });
        label_row("Color", [&] {
            ImGui::ColorEdit3("##ground_color", glm::value_ptr(ground.color));
        });
        label_row("Grid color", [&] {
            ImGui::ColorEdit3("##ground_grid_color",
                              glm::value_ptr(ground.grid_color));
        });
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset")) {
        environment = Environment{};
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Restore environment defaults");
    }
}

} // namespace ivx
