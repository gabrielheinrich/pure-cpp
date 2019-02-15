#pragma once

#include <pure/traits.hpp>
#include <pure/types/some.hpp>

namespace pure {
	template<template<typename O, typename N> typename a,
			template<typename O, typename N> typename b, typename A, typename B, typename Nil_A, typename Nil_B>
	struct Trait_Type_Union<a<A, Nil_A>, b<B, Nil_B>, Type_Class::Var, Type_Class::Var> : Trait_Definition {
		using Nil = std::conditional_t<std::is_same_v<Nil_A, Nil_B>, Nil_A, maybe_nil>;
		using Object_Type = std::conditional_t<std::is_same_v<A, B>, A, Interface::Value>;
		static_assert (a<A, Nil_A>::definitely_pointer && b<B, Nil_B>::definitely_pointer);
		using value = std::conditional_t<std::is_same_v<a<Object_Type, Nil>, b<Object_Type, Nil>>, a<Object_Type, Nil>, some<Object_Type, Nil>>;
	};

	template<typename A, typename B>
	struct Trait_Type_Union<A, B, Type_Class::Var, Type_Class::Var> : Trait_Definition {
		static constexpr bool pointer = A::definitely_pointer && B::definitely_pointer;
		using Object_Type = std::conditional_t<std::is_same_v<Var::object_type<A>, Var::object_type<B>>, Var::object_type<A>, Interface::Value>;
		using Nil = std::conditional_t<A::maybe_nil || B::maybe_nil, maybe_nil, never_nil>;
		using value = std::conditional_t<pointer, some<Object_Type, Nil>, var>;
	};
};