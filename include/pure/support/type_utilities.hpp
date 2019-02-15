#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

namespace pure::detail {
	template<typename T>
	static constexpr bool not_reachable = false;

	inline void not_callable ();

	template<typename T>
	struct type_value { using value = T; };

	template<typename... Ts>
	struct type_list {
		static constexpr intptr_t length = 0;
		static constexpr bool empty = true;
		using first = void;
		using rest = type_list<>;
	};

	template<typename First, typename... Rest>
	struct type_list<First, Rest...> {
		static constexpr intptr_t length = sizeof... (Rest) + 1;
		static constexpr bool empty = false;
		using first = First;
		using rest = type_list<Rest...>;
	};

	template<typename First, typename Second>
	struct type_pair {
		using first = First;
		using second = Second;
	};

	template<typename A, typename B = type_list<>>
	struct pair_type_list {
		static_assert (A::empty);
		using value = B;
	};

	template<typename First, typename Second, typename... Rest, typename... Pairs>
	struct pair_type_list<type_list<First, Second, Rest...>, type_list<Pairs...>> {
		using value = typename pair_type_list<type_list<Rest...>, type_list<Pairs..., type_pair<First, Second>>>::value;
	};

	template<typename T>
	constexpr bool is_moveable = std::is_rvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>;

	template<typename T>
	auto move_or_copy (T&& self) -> std::conditional_t<std::is_rvalue_reference_v<T&&>, T&&, T> {
		if constexpr (is_moveable<T&&>) return std::move (self);
		else return self;
	}

	template<typename A, typename B, typename = void>
	struct equality_comparable_t : std::false_type {};

	template<typename A, typename B>
	struct equality_comparable_t<A, B, decltype (std::declval<A> () == std::declval<B> (), void ())>
			: std::true_type {
	};

	template<typename A, typename B>
	static constexpr bool equality_comparable = equality_comparable_t<A, B>::value;

	template<typename T>
	struct primitive_class {
		primitive_class (T value) : value {value} {};
		operator T () const { return value; }
	private:
		T value;
	};

	template<typename T>
	using generic_base = std::conditional_t<std::is_class_v<T>, T, primitive_class<T>>;

	struct void_class {};

	template<typename T, typename C>
	using inherit_once = std::conditional_t<std::is_base_of_v<C, T>, void_class, C>;
}
