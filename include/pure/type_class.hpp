#pragma once

#include <pure/support/type_utilities.hpp>
#include <utility>
#include <tuple>
#include <cstdio>
#include <iostream>
#include <exception>
#include <cstring>

namespace pure {
	namespace detail {
		struct type_classified {};

		struct auto_t {
			template<typename T>
			operator T () const;
		};

		template<typename T>
		T operator* (T, auto_t);
		template<typename T>
		T operator* (auto_t, T);

		template<typename T>
		T operator+ (T, auto_t);
		template<typename T>
		T operator+ (auto_t, T);

		template<typename T>
		T operator* (T, auto_t);
		template<typename T>
		T operator* (auto_t, T);

		template<typename T>
		T operator/ (T, auto_t);
		template<typename T>
		T operator/ (auto_t, T);

		template<typename T>
		T operator% (T, auto_t);
		template<typename T>
		T operator% (auto_t, T);

		template<typename T>
		bool operator== (T, auto_t);
		template<typename T>
		bool operator== (auto_t, T);

		template<typename T>
		bool operator!= (T, auto_t);
		template<typename T>
		bool operator!= (auto_t, T);

		template<typename T>
		bool operator> (T, auto_t);
		template<typename T>
		bool operator> (auto_t, T);

		template<typename T>
		bool operator< (T, auto_t);
		template<typename T>
		bool operator< (auto_t, T);

		template<typename T>
		bool operator>= (T, auto_t);
		template<typename T>
		bool operator>= (auto_t, T);

		template<typename T>
		bool operator<= (T, auto_t);
		template<typename T>
		bool operator<= (auto_t, T);

		template<typename T, typename = void>
		struct is_iterable : std::false_type {};
		template<typename T>
		struct is_iterable<T, decltype (++std::declval<T> ().begin () == std::declval<T> ().end (), void ())>
				: std::true_type {
		};

		template<typename T, typename = void>
		struct is_random_access_iterable : std::false_type {};
		template<typename T>
		struct is_random_access_iterable<T, decltype (std::declval<T> ().begin () + 1, void ())> : std::true_type {};

		template<typename T, typename = void>
		struct is_bidirectional_iterable : std::false_type {};
		template<typename T>
		struct is_bidirectional_iterable<T, decltype (std::declval<T> ().rbegin (), --std::declval<T> ().begin (), void ())>
				: std::true_type {
		};

		template<typename T, typename = void>
		struct is_sized_iterable : std::false_type {};
		template<typename T>
		struct is_sized_iterable<T, decltype (std::declval<T> ().size (), void ())> : std::true_type {};

		template<typename T, typename = void>
		struct is_contiguous_iterable : std::false_type {};
		template<typename T>
		struct is_contiguous_iterable<T, decltype (std::declval<T> ().data (), void ())> : std::true_type {};

		template<typename T, typename = void>
		struct has_simple_call_operator : std::false_type {};
		template<typename T>
		struct has_simple_call_operator<T, decltype (&T::operator(), void ())> : std::true_type {};

		struct callable_t { void operator() (std::nullptr_t); };

		template<typename T>
		struct add_extra_call_operator : T, callable_t {};

		template<typename T, typename = void>
		struct has_call_operator : std::true_type {};
		template<typename T>
		struct has_call_operator<T, decltype (&add_extra_call_operator<T>::operator(), void ())> : std::false_type {};

		std::false_type check_call (...);
		template<typename F, typename... Args>
		auto check_call (F&& f, Args... args) -> decltype (f (args...));
	}

	namespace Type_Class {
		struct External;

		struct Nil;
		struct Bool;
		struct Int;
		struct Character;
		struct Double;
		struct CString;
		struct Exception;
		struct Std_Stream;
		struct File_Stream;

		template<typename... Elements>
		struct Tuple;

		template<bool Sized, bool Bidirectional, bool Random_Access, bool Contiguous>
		struct Iterable_Capabilities {
			static constexpr bool sized = Sized;
			static constexpr bool bidirectional = Bidirectional;
			static constexpr bool random_access = Random_Access;
			static constexpr bool contiguous = Contiguous;
		};

		template<typename Capabilitiy>
		struct Iterable;

		template<typename R, typename... Args>
		struct Function;

		template<typename R, intptr_t Arity, bool Variadic>
		struct Generic_Function;

		struct Var;
		struct Symbolic_Set;
		struct Identifier;
		struct Sequence;
		struct Stream;

		struct Record;

		struct Restricted;

		template<typename T>
		struct find_primitive { using value = External; };

		template<typename R, typename... Args>
		struct find_primitive<R (Args...)> { using value = Function<R, Args...>; };
		template<typename R, typename... Args>
		struct find_primitive<R(*) (Args...)> { using value = Function<R, Args...>; };
		template<typename R, typename... Args>
		struct find_primitive<R(&) (Args...)> { using value = Function<R, Args...>; };
		template<typename C, typename R, typename... Args>
		struct find_primitive<R(C::*) (Args...)> { using value = Function<R, Args...>; };
		template<typename C, typename R, typename... Args>
		struct find_primitive<R(C::*) (Args...) const> { using value = Function<R, Args...>; };

		template<typename T, typename... Args>
		struct find_generic_callable {
			static constexpr auto eval () {
				static_assert (sizeof... (Args) != 10, "Number of arguments exceeds limit");

				using call = decltype (detail::check_call (std::declval<T> (), std::declval<Args> ()...));
				if constexpr (!std::is_same_v<std::false_type, call>) {
					using call2 = decltype (detail::check_call (std::declval<T> (), detail::auto_t {},
																std::declval<Args> ()...));
					if constexpr (!std::is_same_v<std::false_type, call2>) {
						return detail::type_value<Generic_Function<call, sizeof... (Args), true>> {};
					}
					else {
						return detail::type_value<Generic_Function<call, sizeof... (Args), false>> {};
					}
				}
				else {
					return find_generic_callable<T, detail::auto_t, Args...>::eval ();
				}
			}

		};

		template<typename T>
		struct find {
			static constexpr auto eval () {
				if constexpr (std::is_class<T>::value) {
					if constexpr (std::is_base_of_v<detail::type_classified, T>)
						return detail::type_value<typename T::type_class> {};
					else if constexpr (std::is_base_of_v<std::exception, T>)
						return detail::type_value<Exception> {};
					else if constexpr (std::is_base_of_v<std::ostream, T> || std::is_base_of_v<std::istream, T>)
						return detail::type_value<Std_Stream> {};
					else if constexpr (detail::has_simple_call_operator<T>::value) {
						return detail::type_value<typename find_primitive<decltype (&T::operator())>::value> {};
					}
					else if constexpr (detail::has_call_operator<T>::value) {
						return detail::type_value<typename decltype (find_generic_callable<T>::eval())::value> {};
					}
					else if constexpr (detail::is_iterable<T>::value) {
						return detail::type_value<
								Iterable<Iterable_Capabilities<
										detail::is_sized_iterable<T>::value,
										detail::is_bidirectional_iterable<T>::value,
										detail::is_random_access_iterable<T>::value,
										detail::is_contiguous_iterable<T>::value>>> {};
					}
					else
						return detail::type_value<External> {};
				}
				else {
					return detail::type_value<typename find_primitive<T>::value> {};
				}
			}

			using value = typename decltype (find<T>::eval ())::value;
		};

		template<>
		struct find<std::nullptr_t> { using value = Nil; };
		template<>
		struct find<bool> { using value = Bool; };
		template<>
		struct find<signed char> { using value = Int; };
		template<>
		struct find<short> { using value = Int; };
		template<>
		struct find<int> { using value = Int; };
		template<>
		struct find<long> { using value = Int; };
		template<>
		struct find<long long> { using value = Int; };
		template<>
		struct find<unsigned char> { using value = Int; };
		template<>
		struct find<unsigned short> { using value = Int; };
		template<>
		struct find<unsigned int> { using value = Int; };
		template<>
		struct find<unsigned long> { using value = Int; };
		template<>
		struct find<unsigned long long> { using value = Int; };
		template<>
		struct find<float> { using value = Double; };
		template<>
		struct find<double> { using value = Double; };
		template<>
		struct find<char> { using value = Character; };
		template<>
		struct find<char16_t> { using value = Character; };
		template<>
		struct find<char32_t> { using value = Character; };
		template<>
		struct find<const char*> { using value = CString; };
		template<>
		struct find<char*> { using value = CString; };
		template<>
		struct find<std::FILE*> { using value = File_Stream; };

		template<typename T>
		struct find<const T> { using value = typename find<T>::value; };

		template<typename First, typename Second>
		struct find<std::pair<First, Second>> { using value = Tuple<First, Second>; };

		template<typename... Elements>
		struct find<std::tuple<Elements...>> { using value = Tuple<Elements...>; };

		template<typename Trait, typename T>
		struct find_for {
			using value = typename find<T>::value;
		};
	}

	template<typename T>
	struct implements : detail::type_classified { using type_class = T; };

	template<typename T>
	using type_class = typename Type_Class::find<std::decay_t<T>>::value;

	template<typename Trait, typename T>
	using type_class_for = typename Type_Class::find_for<Trait, std::decay_t<T>>::value;

	struct Trait_Declaration { static constexpr bool implemented = false; };
	struct Trait_Definition { static constexpr bool implemented = true; };
}
