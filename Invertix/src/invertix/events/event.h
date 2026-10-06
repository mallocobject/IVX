#pragma once

#include "ivx_pch.h"
#include "invertix/core.h"

namespace invertix::detail {
	template <typename F>
	struct function_traits;

	// function pointer
	template <typename R, typename Arg>
	struct function_traits<R(*)(Arg)>
	{
		using arg_type = Arg;
	};

	// non-const member function pointer
	template <typename C, typename R, typename Arg>
	struct function_traits<R(C::*)(Arg)>
	{
		using arg_type = Arg;
	};

	// const member function pointer
	template <typename C, typename R, typename Arg>
	struct function_traits<R(C::*)(Arg) const>
	{
		using arg_type = Arg;
	};

	// function object / lambda: operator()
	template <typename F>
	struct function_traits : function_traits<decltype(&F::operator())> {};

	template <typename F>
	using first_arg_t = std::remove_cvref_t<
		typename function_traits<std::decay_t<F>>::arg_type>;
}


namespace invertix {
	enum class EventType : uint8_t
	{
		kNone = 0,
		kWindowClose,
		kWindowResize,
		kWindowFocus,
		kWindowLostFocus,
		kWindowMoved,
		kAppTick,
		kAppUpdate,
		kAppRender,
		kKeyPressed,
		kKeyReleased,
		kKeyTyped,
		kMouseButtonPressed,
		kMouseButtonReleased,
		kMouseMoved,
		kMouseScrolled
	};

	enum class EventCategory : uint8_t
	{
		kNone = 0,
		kEventCategoryApplication = BIT(0),
		kEventCategoryInput = BIT(1),
		kEventCategoryKeyboard = BIT(2),
		kEventCategoryMouse = BIT(3),
		kEventCategoryMouseButton = BIT(4)
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

#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { using enum EventType; return type; }\
								virtual EventType get_event_type() const override { return get_static_type(); }\
								virtual std::string_view get_name() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual EventCategory get_category_flags() const override \
										{ using enum EventCategory; return category; }


	class Event
	{
	public:
		bool handled{ false };

		virtual ~Event() = default;
		virtual EventType get_event_type() const = 0;
		virtual std::string_view get_name() const = 0;
		virtual EventCategory get_category_flags() const = 0;
		virtual std::string to_string() const { return std::string{ get_name() }; }

		bool is_in_category(EventCategory category) const
		{
			return !!(get_category_flags() & category);
		}
	};


	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: event_(event)
		{
		}

		// F will be deduced by the compiler
		template<typename F>
			requires requires (F& f, detail::first_arg_t<F>& e) { { f(e) } -> std::convertible_to<bool>; }
		bool dispatch(const F& func)
		{
			using T = detail::first_arg_t<F>;
			if (event_.get_event_type() == T::get_static_type())
			{
				event_.handled |= func(static_cast<T&>(event_));
				return true;
			}
			return false;
		}

	private:
		Event& event_;
	};

	//inline std::ostream& operator<<(std::ostream& os, const Event& e)
	//{
	//	return os << e.get_name();
	//}
}

template <>
struct std::formatter<invertix::Event, char> : std::formatter<std::string, char> {
	auto format(const invertix::Event& value, std::format_context& ctx) const {
		return std::formatter<std::string, char>::format(value.to_string(), ctx);
	}
};

//#define IVX_EVENT_FORMATTER(Type)                                                  \
//    template <>                                                                    \
//    struct std::formatter<Type, char> : std::formatter<std::string, char> {        \
//        auto format(const Type& value, std::format_context& ctx) const {           \
//            return std::formatter<std::string, char>::format(                      \
//                value.to_string(), ctx);										   \
//        }                                                                          \
//    }
