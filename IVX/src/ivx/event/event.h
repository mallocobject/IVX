#pragma once

#include "ivx/core/base.h"
#include <format>
#include <string>
#include <string_view>

namespace ivx {
namespace {
template <typename F>
struct function_traits;

// function pointer
template <typename R, typename Arg>
struct function_traits<R (*)(Arg)> {
    using arg_type = Arg;
};

// non-const member function pointer
template <typename C, typename R, typename Arg>
struct function_traits<R (C::*)(Arg)> {
    using arg_type = Arg;
};

// const member function pointer
template <typename C, typename R, typename Arg>
struct function_traits<R (C::*)(Arg) const> {
    using arg_type = Arg;
};

// function object / lambda: operator()
template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template <typename F>
using first_arg_t =
    std::remove_cvref_t<typename function_traits<std::decay_t<F>>::arg_type>;
} // namespace

enum class EventType : uint8_t {
    IVX_None = 0,
    IVX_WindowClose,
    IVX_WindowResize,
    IVX_WindowFocus,
    IVX_WindowLostFocus,
    IVX_WindowMoved,
    IVX_AppTick,
    IVX_AppUpdate,
    IVX_AppRender,
    IVX_KeyPressed,
    IVX_KeyReleased,
    IVX_KeyTyped,
    IVX_MouseButtonPressed,
    IVX_MouseButtonReleased,
    IVX_MouseMoved,
    IVX_MouseScrolled
};

enum class EventCategory : uint8_t {
    IVX_None = 0,
    IVX_EventCategoryApplication = BIT(0),
    IVX_EventCategoryInput = BIT(1),
    IVX_EventCategoryKeyboard = BIT(2),
    IVX_EventCategoryMouse = BIT(3),
    IVX_EventCategoryMouseButton = BIT(4)
};

constexpr EventCategory operator&(EventCategory a, EventCategory b) {
    return static_cast<EventCategory>(static_cast<uint8_t>(a) &
                                      static_cast<uint8_t>(b));
}

constexpr EventCategory operator|(EventCategory a, EventCategory b) {
    return static_cast<EventCategory>(static_cast<uint8_t>(a) |
                                      static_cast<uint8_t>(b));
}

constexpr bool operator!(EventCategory m) noexcept {
    return static_cast<uint8_t>(m) == 0;
}

#define EVENT_CLASS_TYPE(type)                                                 \
    static EventType get_static_type() {                                       \
        using enum EventType;                                                  \
        return type;                                                           \
    }                                                                          \
    virtual EventType get_event_type() const override {                        \
        return get_static_type();                                              \
    }                                                                          \
    virtual std::string_view get_name() const override {                       \
        return #type;                                                          \
    }

#define EVENT_CLASS_CATEGORY(category)                                         \
    virtual EventCategory get_category_flags() const override {                \
        using enum EventCategory;                                              \
        return category;                                                       \
    }

class Event {
  public:
    bool handled{false};

    virtual ~Event() = default;
    virtual EventType get_event_type() const = 0;
    virtual std::string_view get_name() const = 0;
    virtual EventCategory get_category_flags() const = 0;
    virtual std::string to_string() const {
        return std::string{get_name()};
    }

    bool is_in_category(EventCategory category) const {
        return !!(get_category_flags() & category);
    }
};

class EventDispatcher {
  public:
    EventDispatcher(Event &event) : event_(event) {
    }

    // F will be deduced by the compiler
    template <typename F>
        requires requires(F &f, first_arg_t<F> &e) {
            { f(e) } -> std::convertible_to<bool>;
        }
    bool dispatch(const F &func) {
        using T = first_arg_t<F>;
        if (event_.get_event_type() == T::get_static_type()) {
            event_.handled |= func(static_cast<T &>(event_));
            return true;
        }
        return false;
    }

  private:
    Event &event_;
};
} // namespace ivx

template <>
struct std::formatter<ivx::Event, char> : std::formatter<std::string, char> {
    auto format(const ivx::Event &value, std::format_context &ctx) const {
        return std::formatter<std::string, char>::format(value.to_string(),
                                                         ctx);
    }
};
