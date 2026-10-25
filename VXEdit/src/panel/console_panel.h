#pragma once

#include "ivx/core/logger.h"
#include "panel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ivx {

// Editor log view. The engine side (bqlog) keeps console output in a buffer;
// this panel drains it every frame (see pump()) and renders it with per-level
// filters, a text filter and auto-scroll.
class ConsolePanel : public Panel {
  public:
    ConsolePanel();

    // Pulls new messages from bqlog's buffer. Called from the layer's update,
    // so messages keep being collected while the panel is closed.
    void pump();

  protected:
    void on_imgui() override;

  private:
    struct Entry {
        LogLevel level{LogLevel::INFO};
        std::uint64_t log_id{0};
        std::string text;
    };

    static constexpr int k_level_count = 6;

    void draw_toolbar();
    void draw_entries();
    bool visible(LogLevel level) const;
    bool matches_filter(const std::string &text) const;

    std::vector<Entry> entries_;
    std::size_t max_entries_{4096};
    std::size_t counts_[k_level_count]{};
    bool level_visible_[k_level_count]{true, true, true, true, true, true};
    bool auto_scroll_{true};
    bool scroll_to_bottom_{false};
    char filter_[128]{};
};

} // namespace ivx
