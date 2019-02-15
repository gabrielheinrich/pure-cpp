#pragma once

#include <pure/support/type_utilities.hpp>
#include <pure/traits.hpp>
#include <pure/types/nil.hpp>

namespace pure::detail {

	namespace transience {
		struct persistent {};
		struct transient {};
		struct resettable_transient {};
		struct some_pointer {};
		struct resettable_pointer {};
		struct generic {};
	}

	template<typename var_type, typename transient_type, typename persistent_type>
	struct transience_path_t {
		using T = std::decay_t<var_type>;
		static constexpr bool moveable = detail::is_moveable<var_type>;

		using value =
		std::conditional_t <!moveable || T::definitely_const_pointer,
			transience::persistent,
			std::conditional_t <T::definitely_mutable_pointer && moveable,
				std::conditional_t <T::template transient_reset_accepts<transient_type>,
					transience::resettable_transient,
					transience::transient>,
				std::conditional_t <T::definitely_pointer,
					std::conditional_t <moveable && T::template transient_reset_accepts<transient_type> &&
										T::template transient_reset_accepts<persistent_type>,
						transience::resettable_pointer,
						transience::some_pointer>,
					transience::generic>>>;
	};

	template<typename var_type, typename transient_type, typename persistent_type>
	using transience_path = typename transience_path_t <var_type, transient_type, persistent_type>::value;

	template<typename path, typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t;

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::persistent, var_type, transient_type, persistent_type, generic_type> {
		using value = persistent_type;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::transient, var_type, transient_type, persistent_type, generic_type> {
		using value = transient_type;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::generic, var_type, transient_type, persistent_type, generic_type> {
		using value = std::conditional_t<std::is_void_v<generic_type>, unify_types<type_without_nil<transient_type>, persistent_type>, generic_type>;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::resettable_transient, var_type, transient_type, persistent_type, generic_type> {
		using value = var_type;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::resettable_pointer, var_type, transient_type, persistent_type, generic_type> {
		using value = var_type;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type>
	struct transience_return_type_t<transience::some_pointer, var_type, transient_type, persistent_type, generic_type> {
		using value = unify_types<type_without_nil<transient_type>, persistent_type>;
	};

	template<typename var_type, typename transient_type, typename persistent_type, typename generic_type = void>
	using transience_return_type = typename transience_return_type_t<transience_path<var_type, transient_type, persistent_type>, var_type, transient_type, persistent_type, generic_type>::value;

	template<typename transient_type, typename source_type>
	type_without_nil<transient_type> transient_return (transient_type&& transient, source_type&& source) {
		if (Var::tag (transient) == Var::Tag::Nil) {
			return std::move (source);
		}
		else {
			return std::move (transient);
		}
	};

	template<typename source_type>
	source_type&& transient_return (nil_t&& transient, source_type&& source) {
		return std::move (source);
	}
}