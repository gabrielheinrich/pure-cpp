#pragma once

#include <utility>
#include <pure/support/type_utilities.hpp>
#include <pure/type_class.hpp>

namespace pure {
	template<char... Characters>
	struct static_id : implements<Type_Class::Identifier> {
		constexpr static_id() = default;
		static constexpr const char string[sizeof... (Characters)] = {Characters...};
		static constexpr intptr_t length = sizeof... (Characters) - 1;
		operator const char* () const noexcept { return string; }
	};

	template<typename T>
	struct is_static_id : std::false_type {};

	template<char... Characters>
	struct is_static_id<static_id<Characters...>> : std::true_type {};
}

#if defined(__GNUC__) && !defined(__ICC)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"

namespace pure {
	template<typename Char, Char... Chars>
	constexpr auto operator ""_id () {
		return pure::static_id<static_cast<char>(Chars)..., '\0'> {};
	}
}

#define STR(str) static_instance<decltype(str ## _id)>::instance
#define STR_t(str) decltype (str ## _id)

#pragma GCC diagnostic pop
#else
// Macro Implementation
// All credit goes to https://github.com/taocpp/PEGTL

/*
The MIT License (MIT)

Copyright (c) 2007-2019 Dr. Colin Hirsch and Daniel Frey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
namespace pure::detail {
	template <typename, typename, typename, typename, typename, typename, typename, typename>
	struct string_join;

	template<template< char... > class S, char... C0s, char... C1s, char... C2s, char... C3s, char... C4s, char... C5s, char... C6s, char... C7s >
	struct string_join < S< C0s... >, S< C1s... >, S< C2s... >, S< C3s... >, S< C4s... >, S< C5s... >, S< C6s... >, S< C7s... > >
	{
		using type = S< C0s..., C1s..., C2s..., C3s..., C4s..., C5s..., C6s..., C7s... >;
	};

	template<int C>
	struct string_at
	{
		using type = static_id<static_cast<char>(C)>;
	};

	template<>
	struct string_at<-1>
	{
		using type = static_id<>;
	};

	template< typename T, std::size_t S >
	struct string_max_length
	{
		static_assert( S <= 64, "String longer than 64 (including terminating \\0)!" );
		using type = T;
	};
}
#define DETAIL_PURE_CPP_EMPTY()
#define DETAIL_PURE_CPP_DEFER(X) X DETAIL_PURE_CPP_EMPTY()
#define DETAIL_PURE_CPP_EXPAND(...) __VA_ARGS__

#define DETAIL_PURE_CPP_STRING_AT(str, n) \
	pure::detail::string_at<(0##n < sizeof (str)) ? (str)[0##n] : -1>::type

#define DETAIL_PURE_CPP_STRING_JOIN_8(Macro, str, n) \
	pure::detail::string_join < DETAIL_PURE_CPP_DEFER (Macro)(str, n##0), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##1), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##2), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##3), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##4), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##5), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##6), \
								DETAIL_PURE_CPP_DEFER (Macro)(str, n##7)> :: type

#define DETAIL_PURE_CPP_STRING_8(str, n) \
	DETAIL_PURE_CPP_STRING_JOIN_8 (DETAIL_PURE_CPP_STRING_AT, str, n)

#define DETAIL_PURE_CPP_STRING_64(str, n) \
	DETAIL_PURE_CPP_STRING_JOIN_8 (DETAIL_PURE_CPP_STRING_8, str, n)

#define DETAIL_PURE_CPP_TYPE_STRING(str) \
	DETAIL_PURE_CPP_EXPAND( \
		DETAIL_PURE_CPP_EXPAND( \
			pure::detail::string_max_length<DETAIL_PURE_CPP_STRING_64(str, 0), sizeof (str)>::type))


#define STR(str) static_instance<DETAIL_PURE_CPP_TYPE_STRING(str)>::instance
#define STR_t(str) DETAIL_PURE_CPP_TYPE_STRING(str)
#endif

