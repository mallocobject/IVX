#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <imgui.h>

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"

namespace ivx {

// ---------------------------------------------------------------------------
// Panel: one editor window.
//
// A subclass only implements on_imgui(), which runs between Begin()/End().
// The pairing is owned by the base class because ImGui requires End() to be
// called even when Begin() returns false (collapsed window).
//
// RefCounted so panels can be shared with the rest of the editor through
// ivx::Ref, like Layer/Texture/Shader/Scene.
// ---------------------------------------------------------------------------
class Panel : public RefCounted {
  public:
    explicit Panel(std::string_view name, bool default_open = true);
    virtual ~Panel() = default;

    Panel(const Panel &) = delete;
    Panel &operator=(const Panel &) = delete;

    void on_render();

    bool &visible_flag() {
        return opened_;
    }
    std::string_view name() const {
        return name_;
    }
    bool is_open() const {
        return opened_;
    }
    void set_open(bool open) {
        opened_ = open;
    }
    bool visible_this_frame() const {
        return visible_this_frame_;
    }

    // Size used the first time the window appears (ignored once ImGui has a
    // saved layout for it). Docked windows ignore it as well.
    Panel &set_default_size(ImVec2 size) {
        default_size_ = size;
        return *this;
    }

    // Extra flags for the window itself, e.g. ImGuiWindowFlags_NoDocking for
    // panels that float instead of taking a dock slot (the stats overlay).
    Panel &set_window_flags(ImGuiWindowFlags flags) {
        window_flags_ = flags;
        return *this;
    }

  protected:
    virtual void on_imgui() = 0;

    // Called once, right before the first draw.
    virtual void on_first_open() {
    }

    // Called every frame; `visible` is false when the panel is closed or
    // collapsed. Panels that publish state (e.g. viewport size) reset it here.
    virtual void on_after_draw(bool /*visible*/) {
    }

    // Overridable so panels can push style vars around the window itself.
    virtual bool begin_window();
    virtual void end_window();

    // Panels that should disappear without being "closed" (the stats overlay
    // only makes sense while the viewport is on screen) override this.
    // Returning false skips Begin/End entirely for the frame.
    virtual bool should_draw() const {
        return opened_;
    }

    std::string name_;
    bool opened_{true};
    ImVec2 default_size_{0.f, 0.f};
    ImGuiWindowFlags window_flags_{0};

  private:
    bool first_open_done_{false};
    bool visible_this_frame_{false};
};

using PanelContent = std::function<void()>;

// ---------------------------------------------------------------------------
// LambdaPanel: adapter for panels that are still a plain block of ImGui calls.
// Lets existing inline editor windows move into the manager without a rewrite.
// ---------------------------------------------------------------------------
class LambdaPanel : public Panel {
  public:
    LambdaPanel(std::string_view name,
                PanelContent content,
                bool default_open = true);

    LambdaPanel &set_pre_begin(PanelContent fn) {
        pre_begin_ = std::move(fn);
        return *this;
    }
    LambdaPanel &set_post_end(PanelContent fn) {
        post_end_ = std::move(fn);
        return *this;
    }
    LambdaPanel &set_on_hidden(PanelContent fn) {
        on_hidden_ = std::move(fn);
        return *this;
    }

  protected:
    bool begin_window() override;
    void end_window() override;
    void on_imgui() override;
    void on_after_draw(bool visible) override;

  private:
    PanelContent content_;
    PanelContent pre_begin_;
    PanelContent post_end_;
    PanelContent on_hidden_;
};

// ---------------------------------------------------------------------------
// PanelManager: owns the panels, draws them in registration order and provides
// the View menu that toggles their visibility.
//
// Panels are held through ivx::Ref, so handing one out (to EditorContext, to
// another panel, ...) never leaves a dangling pointer.
// ---------------------------------------------------------------------------
class PanelManager {
  public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<Panel, T>
    Ref<T> add(Args &&...args) {
        Ref<T> panel = make_ref<T>(std::forward<Args>(args)...);
        panels_.push_back(panel); // Ref<T> -> Ref<Panel>
        return panel;
    }

    Ref<Panel> add(Ref<Panel> panel);

    void on_render();

    // Must be called from inside a menu bar.
    void draw_view_menu(const char *menu_name = "View");

    template <typename T>
    Ref<T> find() {
        if constexpr (std::is_base_of_v<Panel, T>) {
            for (auto &panel : panels_) {
                if (auto *typed = dynamic_cast<T *>(panel.get())) {
                    return Ref<T>{typed};
                }
            }
        }
        return {};
    }

    std::size_t size() const {
        return panels_.size();
    }

    void clear() {
        panels_.clear();
    }

    std::vector<Ref<Panel>> &panels() {
        return panels_;
    }

  private:
    std::vector<Ref<Panel>> panels_;
};

} // namespace ivx
