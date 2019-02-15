#pragma once

#include <pure/traits.hpp>

namespace pure {
	namespace detail {
		template<typename T>
		struct var_without_nil { using value = T; };

		template<template<typename O, typename Nil> typename v, typename O>
		struct var_without_nil<v<O, maybe_nil>> { using value = v<O, never_nil>; };
	}

	template<typename T>
	struct Trait_Type_Variants<T, Type_Class::Var> : Trait_Definition {
		using without_nil = typename detail::var_without_nil<T>::value;
	};
}