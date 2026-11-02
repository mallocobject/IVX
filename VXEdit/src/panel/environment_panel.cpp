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
    if (ImGui::Button("Reset")) {
        environment = Environment{};
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Back to the default ambient light, curve and exposure");
    }
}

} // namespace ivx
