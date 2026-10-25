#include "panel.h"

namespace ivx {

// ---------------------------------------------------------------------------
// Panel
// ---------------------------------------------------------------------------
Panel::Panel(std::string_view name, bool default_open)
    : name_(name), opened_(default_open) {
}

void Panel::on_render() {
    if (!opened_) { // closed through the title bar or the View menu
        on_after_draw(false);
        return;
    }

    if (!first_open_done_) {
        first_open_done_ = true;
        on_first_open();
    }

    visible_this_frame_ = begin_window();
    if (visible_this_frame_) {
        on_imgui();
    }
    end_window();

    on_after_draw(visible_this_frame_);
}

bool Panel::begin_window() {
    return ImGui::Begin(name_.c_str(), &opened_);
}

void Panel::end_window() {
    ImGui::End();
}

// ---------------------------------------------------------------------------
// LambdaPanel
// ---------------------------------------------------------------------------
LambdaPanel::LambdaPanel(std::string_view name,
                         PanelContent content,
                         bool default_open)
    : Panel(name, default_open), content_(std::move(content)) {
}

bool LambdaPanel::begin_window() {
    if (pre_begin_) {
        pre_begin_();
    }
    return Panel::begin_window();
}

void LambdaPanel::end_window() {
    Panel::end_window();
    if (post_end_) {
        post_end_();
    }
}

void LambdaPanel::on_imgui() {
    if (content_) {
        content_();
    }
}

void LambdaPanel::on_after_draw(bool visible) {
    if (!visible && on_hidden_) {
        on_hidden_();
    }
}

// ---------------------------------------------------------------------------
// PanelManager
// ---------------------------------------------------------------------------
Ref<Panel> PanelManager::add(Ref<Panel> panel) {
    panels_.push_back(std::move(panel));
    return panels_.back();
}

void PanelManager::on_render() {
    for (auto &panel : panels_) {
        panel->on_render();
    }
}

void PanelManager::draw_view_menu(const char *menu_name) {
    if (!ImGui::BeginMenu(menu_name)) {
        return;
    }
    for (auto &panel : panels_) {
        ImGui::MenuItem(panel->name().data(), nullptr, &panel->visible_flag());
    }
    ImGui::EndMenu();
}

} // namespace ivx
