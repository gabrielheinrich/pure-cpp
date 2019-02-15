#pragma once

#include <type_traits>
#include <cassert>
#include <pure/traits.hpp>
#include <pure/type_class.hpp>
#include <pure/symbolic_sets.hpp>
#include <pure/exceptions.hpp>

namespace pure {
	template<typename T, typename Domain>
	struct restricted : detail::generic_base<T>, detail::inherit_once<T, detail::type_classified> {
		using type_class = pure::type_class<T>;
		using base = detail::generic_base<T>;

		template<typename U>
		restricted (U&& other) : base {std::forward<U> (other)} {
			using disjunct_t = pure::definitely_disjunct_t<Domain, pure::domain_t<U>>;
			static_assert (! disjunct_t::eval ());
			if (!Domain {} (static_cast<const T&>(*this))) throw operation_not_supported {};
		}
	};

	/**
	 Restricts a type to a new domain. This can for example be used on value holder types to be more specific about
	 the expected value range.
	 @tparam T type to restrict
	 @tparam Domain domain of new type
	 */
	template<typename T, const auto& Domain>
	using restrict = restricted<T, deref_symbolic_set<decltype (Domain)>>;

	namespace Type_Class {
		template<typename T, typename Domain>
		struct find_for<Trait_Value_Tag, restricted<T, Domain>> { using value = Restricted; };
	}
}
