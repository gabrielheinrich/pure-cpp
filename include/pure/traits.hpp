#pragma once

#include <cstdint>
#include <cassert>
#include <pure/support/type_utilities.hpp>
#include <pure/type_class.hpp>
#include <pure/exceptions.hpp>
#include <cstdarg>
#include <cstdio>

namespace pure {
	//==============================================================================
	//  Value
	//==============================================================================
	struct Trait_Value_Tag;

	template<typename T, typename Type_Class = type_class_for<Trait_Value_Tag, T>>
	struct Trait_Value : Trait_Declaration {};

	template<typename T>
	constexpr int category_id (const T& self) {
		static_assert (Trait_Value<T>::implemented);
		return Trait_Value<T>::category_id (self);
	}

	template<typename T> using domain_t = typename Trait_Value<std::decay_t<T>>::domain_t;

	/**
	Every type is associated with a domain. The domain of a type is a set of all values that objects of that type can
	 model. Domains are used in value based static checking. This templated constexpr queries the domain of a type.

	 */
	template<typename T> constexpr domain_t<T> domain {};

	//==============================================================================
	//  Static Disjunct
	//==============================================================================
	template<typename A, typename B>
	struct Trait_Static_Disjunct : Trait_Declaration {};

	template<typename A, typename B>
	struct definitely_disjunct_t {
		static constexpr bool eval () {
			if constexpr (Trait_Static_Disjunct<A, B>::implemented)
				return Trait_Static_Disjunct<A, B>::eval ();
			else if constexpr (Trait_Static_Disjunct<B, A>::implemented)
				return Trait_Static_Disjunct<B, A>::eval ();
			else
				return false;
		}
	};

	template<typename A>
	struct definitely_disjunct_t<A, A> {
		static constexpr bool eval () { return false; }
	};

	/**
	Statically checks whether or not two sets are definitely disjunct. Can be used to catch 'type' errors, based on
	the  domains of two types.
	 */
	template<const auto& A, const auto& B> constexpr bool definitely_disjunct = definitely_disjunct_t<std::decay_t<decltype (A)>, std::decay_t<decltype (B)>>::eval ();

	//==============================================================================
	//  Type Union
	//==============================================================================
	template<typename A, typename B, typename Type_Class_A = type_class<A>, typename Type_Class_B = type_class<B>>
	struct Trait_Type_Union : Trait_Declaration {};

	struct var;
	template<typename... Types>
	struct unify_types_t { using value = var; };

	template<typename A>
	struct unify_types_t<A> { using value = A; };

	template<typename A, typename... Types>
	struct unify_types_t<A, A, Types...> { using value = typename unify_types_t<A, Types...>::value; };

	template<typename A, typename B, typename... Types>
	struct unify_types_t<A, B, Types...> {
		static constexpr auto eval () {
			if constexpr (Trait_Type_Union<A, B>::implemented)
				return detail::type_value<typename Trait_Type_Union<A, B>::value> {};
			else if constexpr (Trait_Type_Union<B, A>::implemented)
				return detail::type_value<typename Trait_Type_Union<B, A>::value> {};
			else
				return detail::type_value<var> {};
		}
		using value = typename decltype (unify_types_t<A,B, Types...>::eval ())::value;
	};

	/**
	Type utility, which returns a unified type of the argument types. The returned type can be initialized from var
	type in the arguments.
	 */
	template<typename... Types> using unify_types = typename unify_types_t<Types...>::value;

	//==============================================================================
	//  Type Variants
	//==============================================================================
	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_Type_Variants : Trait_Declaration {};

	template<typename T> using type_without_nil = std::conditional_t<Trait_Type_Variants<T>::implemented, typename Trait_Type_Variants<T>::without_nil, T>;

	//==============================================================================
	//  To Var
	//==============================================================================
	struct Trait_To_Var_Tag;

	template<typename T, typename Type_Class = type_class_for<Trait_To_Var_Tag, T>>
	struct Trait_To_Var : Trait_Declaration {};

	namespace Var {
		enum class Tag {
			Nil = 0, Unique, Shared, Moveable, Interned, Weak, False, True, Int, Int64, Double, Char, String, External
		};

#define Var_Tag_Pointer Var::Tag::Unique : case Var::Tag::Shared : case Var::Tag::Moveable :\
            case Var::Tag::Interned : case Var::Tag::Weak

#define Var_Tag_Mut_Pointer Var::Tag::Unique : case Var::Tag::Moveable
#define Var_Tag_Const_Pointer Var::Tag::Shared : case Var::Tag::Interned : case Var::Tag::Weak

		template<typename T> using object_type = typename Trait_To_Var<std::decay_t<T>>::object_type;

		template<typename O, typename T> constexpr bool supports_object_type = std::is_base_of_v<O, object_type<T>>;

		template<typename T> constexpr bool has_interned = Trait_To_Var<std::decay_t<T>>::has_interned;

		template<typename O, typename T> constexpr bool has_interned_for =
				supports_object_type<O, T> && has_interned<T>;

		template<typename T>
		Tag tag (const T& self) noexcept { return Trait_To_Var<T>::tag (self); }

		template<typename O, typename T>
		Tag tag_for (const T& self) noexcept {
			if constexpr (supports_object_type<O, T>)
				return tag (self);
			else {
				auto t = tag (self);
				switch (t) {
					case Var_Tag_Pointer : return Tag::External;
					default : return t;
				}
			}
		};

		template<typename T>
		auto* obj (const T& self) { return Trait_To_Var<T>::obj (self); }
		template<typename T>
		auto* release (T& self) { return Trait_To_Var<std::decay_t<T>>::release (self); }
		template<typename T>
		intptr_t get_int (const T& self) { return Trait_To_Var<T>::get_int (self); }
		template<typename T>
		int64_t get_int64 (const T& self) { return Trait_To_Var<T>::get_int64 (self); }
		template<typename T>
		double get_double (const T& self) { return Trait_To_Var<T>::get_double (self); }
		template<typename T>
		char32_t get_char (const T& self) { return Trait_To_Var<T>::get_char (self); }
		template<typename T>
		const char* get_cstring (const T& self) { return Trait_To_Var<T>::get_cstring (self); }
		template<typename T>
		intptr_t get_cstring_length (const T& self) { return Trait_To_Var<T>::get_cstring_length (self); }

		template<typename T>
		auto* clone (T&& self) { return Trait_To_Var<std::decay_t<T>>::clone (std::forward<T> (self)); }

		template<typename T>
		intptr_t clone_bytes_needed (const T& self) { return Trait_To_Var<T>::clone_bytes_needed (self); }
		template<typename T>
		auto* clone_placement (T&& self, void* position, intptr_t num_bytes) {
			return Trait_To_Var<std::decay_t<T>>::clone_placement (std::forward<T> (self), position, num_bytes);
		}
	}

	//==============================================================================
	//  From Var
	//==============================================================================
	struct Trait_From_Var_Tag;

	template<typename T, typename Type_Class = type_class_for<Trait_From_Var_Tag, T>>
	struct Trait_From_Var : Trait_Declaration {
		static constexpr bool to_ref_implemented = false;
	};

	//==============================================================================
	//  Compare
	//==============================================================================
	struct Trait_Compare_Tag;

	template<typename LHS, typename RHS, typename LHS_Type_Class = type_class_for<Trait_Compare_Tag, LHS>, typename RHS_Type_Class = type_class_for<Trait_Compare_Tag, RHS>>
	struct Trait_Compare : Trait_Declaration { static constexpr bool comparable = false; };

	namespace detail {
		template<typename T, typename = void>
		struct Trait_Compare_implements_not_equivalent : std::false_type {};
		template<typename T>
		struct Trait_Compare_implements_not_equivalent<T, decltype (T::not_equivalent, void ())> : std::true_type {};
		template<typename T, typename = void>
		struct Trait_Compare_implements_not_equal : std::false_type {};
		template<typename T>
		struct Trait_Compare_implements_not_equal<T, decltype (T::not_equal, void ())> : std::true_type {};

		template<typename T>
		struct builtin_equivalent : std::false_type {};
		template<>
		struct builtin_equivalent<Type_Class::External> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::Nil> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::Bool> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::Int> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::Double> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::Character> : std::true_type {};
		template<>
		struct builtin_equivalent<Type_Class::CString> : std::true_type {};
	}

	/**
	Strict equality comparison between values. Returns true if x equals y, false if not. Works accross all types.
	Notice the difference with operator== which coerces primitives, e.g. true == 1.
	 */
	constexpr auto equal = [] (const auto& lhs, const auto& rhs) {
		using LHS = std::decay_t<decltype (lhs)>;
		using RHS = std::decay_t<decltype (rhs)>;
		if constexpr (Trait_Compare<LHS, RHS>::implemented)
			return Trait_Compare<LHS, RHS>::equal (lhs, rhs);
		else if constexpr (Trait_Compare<RHS, LHS>::implemented)
			return Trait_Compare<RHS, LHS>::equal (rhs, lhs);
		else return false;
	};

	/**
	Same as ! equal (lhs, rhs)
	 */
	constexpr auto not_equal = [] (const auto& lhs, const auto& rhs) {
		using LHS = std::decay_t<decltype (lhs)>;
		using RHS = std::decay_t<decltype (rhs)>;
		if constexpr (detail::Trait_Compare_implements_not_equal<Trait_Compare<LHS, RHS>>::value)
			return Trait_Compare<LHS, RHS>::not_equal (lhs, rhs);
		else if constexpr (detail::Trait_Compare_implements_not_equal<Trait_Compare<RHS, LHS>>::value)
			return Trait_Compare<RHS, LHS>::not_equal (rhs, lhs);
		else
			return !equal (lhs, rhs);
	};

	/**
	operator== is extended to work accross all types. The comparison is not strict equality, since primitive values are
	 coerced to a common type, i.e. 1 == true. Otherwise the behavior is the same as equal
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator== (const LHS& lhs, const RHS& rhs) {
		if constexpr (Trait_Compare<LHS, RHS>::implemented)
			return Trait_Compare<LHS, RHS>::equivalent (lhs, rhs);
		else if constexpr (Trait_Compare<RHS, LHS>::implemented)
			return Trait_Compare<RHS, LHS>::equivalent (rhs, lhs);
		else
			return false;
	};

	/**
	 Same as lhs == rhs. Can be used as an argument to functions, while operator== can't
	 */
	constexpr auto equivalent = [] (const auto& lhs, const auto& rhs) {
		if constexpr (detail::equality_comparable<decltype (lhs), decltype (rhs)>)
			return lhs == rhs;
		else
			throw operation_not_supported ();
	};

	/**
	operator != is extended to work across all types. Same as ! (lhs == rhs).
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator!= (const LHS& lhs, const RHS& rhs) {
		if constexpr (detail::Trait_Compare_implements_not_equivalent<Trait_Compare < LHS, RHS>>::value)
		return Trait_Compare<LHS, RHS>::not_equivalent (lhs, rhs);
		else if constexpr (detail::Trait_Compare_implements_not_equivalent<Trait_Compare < RHS, LHS>>::value)
		return Trait_Compare<RHS, LHS>::not_equivalent (rhs, lhs);
		else
		return !(lhs == rhs);
	};

	/**
	Lexicographically compares two values with strict equality. Returns 1 if lhs > rhs, -1 if lhs < rhs and 0 if equal
	 (lhs, rhs). If two values are from different categories there's a well-defined but arbitrarily chosen ordering.
	 */
	constexpr auto compare = [] (const auto& lhs, const auto& rhs) -> int {
		using LHS = std::decay_t<decltype (lhs)>;
		using RHS = std::decay_t<decltype (rhs)>;
		if constexpr (Trait_Compare<LHS, RHS>::comparable)
			return Trait_Compare<LHS, RHS>::compare (lhs, rhs);
		else if constexpr (Trait_Compare<RHS, LHS>::comparable)
			return -Trait_Compare<RHS, LHS>::compare (rhs, lhs);
		else {
			assert (category_id (lhs) != category_id (rhs));
			return category_id (lhs) < category_id (rhs) ? -1 : 1;
		}
	};

	/**
	Lexicographically compares two values with relaxed equality. Returns 1 if lhs > rhs, -1 if lhs < rhs and 0 if
	 lhs == rhs. If two values are from different categories there's a well-defined but arbitrarily chosen ordering.
	 */
	constexpr auto equivalent_compare = [] (const auto& lhs, const auto& rhs) -> int {
		using LHS = std::decay_t<decltype (lhs)>;
		using RHS = std::decay_t<decltype (rhs)>;
		if constexpr (std::is_arithmetic_v<LHS> && std::is_arithmetic_v<RHS>)
			return lhs < rhs ? -1 : lhs == rhs ? 0 : 1;
		else if constexpr (Trait_Compare<LHS, RHS>::comparable)
			return Trait_Compare<LHS, RHS>::equivalent_compare (lhs, rhs);
		else if constexpr (Trait_Compare<RHS, LHS>::comparable)
			return -Trait_Compare<RHS, LHS>::equivalent_compare (rhs, lhs);
		else {
			auto cl = category_id (lhs);
			auto cr = category_id (rhs);
			return cl < cr ? -1 : cl > cr ? 1 : throw operation_not_supported ();
		}
	};

	/**
	 operator < is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator< (const LHS& lhs, const RHS& rhs) { return equivalent_compare (lhs, rhs) < 0; }

	/**
	 operator > is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator> (const LHS& lhs, const RHS& rhs) { return equivalent_compare (lhs, rhs) > 0; }

	/**
	 operator <= is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator<= (const LHS& lhs, const RHS& rhs) { return equivalent_compare (lhs, rhs) <= 0; }

	/**
	 operator >= is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<!(
			detail::builtin_equivalent<type_class<LHS>>::value && detail::builtin_equivalent<type_class<RHS>>::value) >>
	bool operator>= (const LHS& lhs, const RHS& rhs) { return equivalent_compare (lhs, rhs) >= 0; }

	//==============================================================================
	//  Arithmetic
	//==============================================================================
	struct Trait_Arithmetic_Tag;

	template<typename LHS, typename RHS, typename LHS_Type_Class = type_class_for<Trait_Arithmetic_Tag, LHS>, typename RHS_Type_Class = type_class_for<Trait_Arithmetic_Tag, RHS>>
	struct Trait_Arithmetic : Trait_Declaration {};

	/**
	 operator + is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<Trait_Arithmetic<LHS, RHS>::implemented>>
	auto operator+ (const LHS& lhs, const RHS& rhs) -> decltype (Trait_Arithmetic<LHS, RHS>::add (lhs, rhs)) {
		return Trait_Arithmetic<LHS, RHS>::add (lhs, rhs);
	};

	/**
	 operator - is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<Trait_Arithmetic<LHS, RHS>::implemented>>
	auto operator- (const LHS& lhs, const RHS& rhs) -> decltype (Trait_Arithmetic<LHS, RHS>::sub (lhs, rhs)) {
		return Trait_Arithmetic<LHS, RHS>::sub (lhs, rhs);
	};

	/**
	 operator * is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<Trait_Arithmetic<LHS, RHS>::implemented>>
	auto operator* (const LHS& lhs, const RHS& rhs) -> decltype (Trait_Arithmetic<LHS, RHS>::mul (lhs, rhs)) {
		return Trait_Arithmetic<LHS, RHS>::mul (lhs, rhs);
	};

	/**
	 operator / is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<Trait_Arithmetic<LHS, RHS>::implemented>>
	auto operator/ (const LHS& lhs, const RHS& rhs) -> decltype (Trait_Arithmetic<LHS, RHS>::div (lhs, rhs)) {
		return Trait_Arithmetic<LHS, RHS>::div (lhs, rhs);
	};

	/**
	 operator % is extended to work accross all types.
	 */
	template<typename LHS, typename RHS, typename = std::enable_if_t<Trait_Arithmetic<LHS, RHS>::implemented>>
	auto operator% (const LHS& lhs, const RHS& rhs) -> decltype (Trait_Arithmetic<LHS, RHS>::mod (lhs, rhs)) {
		return Trait_Arithmetic<LHS, RHS>::mod (lhs, rhs);
	};

	//==============================================================================
	//  Functional
	//==============================================================================
	struct Trait_Functional_Tag;
	template<typename T, typename Type_Class = type_class_for<Trait_Functional_Tag, T>>
	struct Trait_Functional : Trait_Declaration {
		template <typename... Args>
		static constexpr bool Applicable = false;
		template <typename... Args>
		static void apply (Args&&...) {throw operation_not_supported();}
	};

	template<typename T, typename... Args> constexpr bool Applicable = Trait_Functional<T>::template Applicable<Args...>;

	/**
	Applies some arguments to a value. For functions and sets this is equivalent to operator (). For vectors and
	 strings apply is equivalent to nth, i.e. they are interpreted as functions from Int to some target set.
	 */
	template<typename T, typename... Args>
	auto apply (const T& self, Args&& ... args) -> decltype (Trait_Functional<T>::apply (self, std::forward<Args>(args)...)) {
		return Trait_Functional<T>::apply (self, std::forward<Args> (args)...);
	};

	/**
	Returns the arity of a functional value. The arity is the number of arguments, that the function accepts. If the
	 function is variadic, this is the minimum number of arguments that the function requires.
	 */
	template<typename T>
	intptr_t arity (const T& self) { return Trait_Functional<T>::arity (self); }

	/**
	Set of all variadic functional values. Returns true if the value is variadic, false otherwise. A variadic
	 function accepts more than one specific number of arguments.
	 */
	template<typename T>
	bool Variadic (const T& self) noexcept {
		if constexpr (Trait_Functional<T>::implemented)
			return Trait_Functional<T>::Variadic (self);
		else return false;
	}

	/**
	 Returns a new value that's equal to self, except with some key or tuple of keys mapped to a value.
	 @param self The original functional value.
	 @param args  Has the form keys..., value.
	 */
	template<typename T, typename... Args>
	auto set (T&& self, Args&& ... args) -> decltype (Trait_Functional<std::decay_t<T>>::set (std::forward<T> (self),
																							  std::forward<Args> (
																									  args)...)) {
		return Trait_Functional<std::decay_t<T>>::set (std::forward<T> (self), std::forward<Args> (args)...);
	};

	/**
	 Returns a new value in which a key or tuple of keys is removed from the mapping.
	 @param self The original function value.
	 @param args The key or key tuple to remove from the mapping.
	 */
	template<typename T, typename... Args>
	auto
	without (T&& self, Args&& ... args) -> decltype (Trait_Functional<std::decay_t<T>>::without (std::forward<T> (self),
																								 std::forward<Args> (
																										 args)...)) {
		return Trait_Functional<std::decay_t<T>>::without (std::forward<T> (self), std::forward<Args> (args)...);
	};

	//==============================================================================
	//  Enumerable
	//==============================================================================
	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_Enumerable : Trait_Declaration {};

	/**
	 Set of all Enumerable values. Returns true if self is enumerable, false otherwise.
	 */
	template<typename T>
	bool Enumerable (const T& self) {
		if constexpr (Trait_Enumerable<T>::implemented) return Trait_Enumerable<T>::Enumerable (self);
		else return false;
	}

	/**
	 Returns an enumerator, which can be used to iterate over the input value. The enumerator will be bound to the
	 lifetime of self, so never return the enumerator by itself from a function. An enumerator is an abstraction
	 around a loop and can be conceptulized as an iterator pair in traditional C++.
	 @param self :: Enumerable
	 */
	template<typename T>
	auto enumerate (T&& self) { return Trait_Enumerable<std::decay_t<T>>::enumerate (std::forward<T> (self)); }

	/**
	 Number of items in self.
	 */
	template<typename T>
	constexpr intptr_t count (const T& self) { return Trait_Enumerable<T>::count (self); }

	/**
	 Set of all Empty values. Same as count(self) == 0.
	 */
	template<typename T>
	bool Empty (const T& self) { return Trait_Enumerable<T>::Empty (self); }

	/**
	 Returns the nth item in the enumeration of self. nth works on strings, vectors and enumerable functions and sets.
	 @param self :: Enumerable
	 @param n The index of the item to return
	 */
	template<typename T>
	auto nth (T&& self, intptr_t n) -> decltype (Trait_Enumerable<std::decay_t<T>>::nth (std::forward<T> (self), n)) {
		return Trait_Enumerable<std::decay_t<T>>::nth (std::forward<T> (self), n);
	}

	/**
	 Same as nth(self, 0), but can return a more specific type, if self is typed per index, e.g. if self is a tuple.
	 @param self :: Enumerable
	 */
	template<typename T>
	auto first (T&& self) -> decltype (Trait_Enumerable<std::decay_t<T>>::first (std::forward<T> (self))) {
		return Trait_Enumerable<std::decay_t<T>>::first (std::forward<T> (self));
	}

	/**
	 Same as nth(self, 1), but can return a more specific type, if self is typed per index, e.g. if self is a tuple.
	 @param self :: Enumerable
	 */
	template<typename T>
	auto second (T&& self) -> decltype (Trait_Enumerable<std::decay_t<T>>::second (std::forward<T> (self))) {
		return Trait_Enumerable<std::decay_t<T>>::second (std::forward<T> (self));
	}

	/**
	 Returns a new value, in which self is appended with arg. Works on strings and vectors.
	 @param self :: Vector | String
	 */
	constexpr auto append = [] (auto&& self,
								auto&& arg) -> decltype (Trait_Enumerable<std::decay_t<decltype (self)>>::append (
			std::forward<decltype (self)> (self), std::forward<decltype (arg)> (arg))) {
		return Trait_Enumerable<std::decay_t<decltype (self)>>::append (std::forward<decltype (self)> (self),
																		std::forward<decltype (arg)> (arg));
	};


	//==============================================================================
	//  CString
	//==============================================================================
	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_CString : Trait_Declaration {};

	/*
	 Returns a null-terminated utf8 byte string of type const char*, which can be used when interfacing with other APIs.
	 The lifetime of the returned pointer is bound by the lifetime of self.
	 @param self :: String
	 */
	template<typename T>
	const char* raw_cstring (const T& self) { return Trait_CString<T>::raw_cstring (self); }

	/*
	 Same as std::strlen (raw_cstring (self))
	 @param self :: String
	 */
	template<typename T>
	intptr_t raw_cstring_length (const T& self) { return Trait_CString<T>::raw_cstring_length (self); }

	//==============================================================================
	//  Error
	//==============================================================================
	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_Error : Trait_Declaration {};

	/**
	 Returns the error message for self as a null-terminated utf8 byte string. Requires that self is actually an Error.
	 @param self :: Error
	 */
	template<typename T>
	const char* error_message (const T& self) noexcept { return Trait_Error<T>::error_message (self); }

	//==============================================================================
	//  Stream
	//==============================================================================
	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_Stream : Trait_Declaration {};

	namespace IO {
		/**
		 Outputs a byte to a stream
		 @param self The stream to write to
		 @param byte The byte to write to the stream
		 @return true on success, false on failure
		 */
		template<typename T>
		bool write_raw_byte (T& self, uint8_t byte) { return Trait_Stream<T>::write_raw_byte (self, byte); }

		/**
		 Outputs multiple bytes to a stream in a single operation.
		 @param self The stream to write to
		 @param data A pointer to an array of bytes
		 @param num_bytes Number of bytes to read from data and to write to the stream.
		 @return true on success, false on failure
		 */
		template<typename T>
		bool write_raw_bytes (T& self, const uint8_t* data, intptr_t num_bytes) {
			return Trait_Stream<T>::write_raw_bytes (self, data, num_bytes);
		}

		/**
		 Outputs a null-terminated byte string to a stream.
		 @param self The stream to write to
		 @param byte The null-terminated byte string to write to the stream.
		 @return true on success, false on failure
		 */
		template<typename T>
		bool write_raw_string (T& self, const char* data) {
			return Trait_Stream<T>::write_raw_string (self, data);
		}


		/**
		 Formatted output to a stream as if via std::printf
		 @param self The stream to write to
		 @param format The format string to use
		 @param ... The arguments to be used for formatting
		 @return true on success, false on failure
		 */
		template<typename T>
		bool printf_to (T& self, const char* format, ...) {
			va_list args;
			va_start (args, format);
			auto result = Trait_Stream<T>::vprintf (self, format, args);
			va_end (args);
			return result;
		}

		template<typename T>
		void flush (T& self) { Trait_Stream<T>::flush (self); }

		template<typename T>
		int64_t stream_get_position (T& self) { return Trait_Stream<T>::get_position (self); }

		template<typename T>
		bool stream_set_position (T& self, int64_t position) { return Trait_Stream<T>::set_position (self, position); }

		//-1 if unknown
		template<typename T>
		int64_t stream_total_num_bytes (T& self) { return Trait_Stream<T>::total_num_bytes (self); }

		//-1 if unknown
		template<typename T>
		int64_t stream_num_bytes_remaining (T& self) { return Trait_Stream<T>::num_bytes_remaining (self); }

		template<typename T>
		bool stream_is_exhausted (T& self) { return Trait_Stream<T>::is_exhausted (self); }

		//-1 on Failure
		template<typename T>
		intptr_t read_raw_bytes (T& self, uint8_t* buffer, intptr_t buffer_size) {
			return Trait_Stream<T>::read_raw_bytes (self, buffer, buffer_size);
		}

		// 0 if exhausted
		template<typename T>
		uint8_t read_raw_byte (T& self) { return Trait_Stream<T>::read_raw_byte (self); }
	}

	//==============================================================================
	//  Print
	//==============================================================================
	struct Trait_Print_Tag;

	template<typename T, typename Type_Class = type_class_for<Trait_Print_Tag, T>>
	struct Trait_Print : Trait_Declaration {};

	namespace IO {
		//TODO return bool
		/**
		 Prints a value to a stream. Works on atomic and enumerable values. The format is JSON inspired. If self is a
		 string or character it won't be quoted.
		 @param stream The stream to print to
		 @param self The value to print to the stream.
		 */
		template<typename T, typename Stream>
		void print_to (Stream&& stream, const T& self) {
			Trait_Print<T>::print_to (stream, self);
		}

		/**
		 Prints a value to stdout. Same as IO::print_to (stdout, self);
		 */
		template<typename T>
		void print (const T& self) { IO::print_to (stdout, self); }
	}

	//==============================================================================
	//  Hash
	//==============================================================================
	struct Trait_Hash_Tag;

	template<typename T, typename Type_Class = type_class_for<Trait_Hash_Tag, T>>
	struct Trait_Hash : Trait_Declaration {};

	/**
	 Returns a 32bit integer hash of a value, which satisfies
	 equal (a, b) => hash(a) == hash(b)
	 */
	constexpr auto hash = [] (const auto& self) -> int32_t {
		return Trait_Hash<std::decay_t<decltype (self)>>::hash (self);
	};

	//==============================================================================
	//  Object
	//==============================================================================

	template<typename T, typename Type_Class = type_class<T>>
	struct Trait_Object : Trait_Definition {
		template<typename To>
		struct obj_cast {
			template<typename T_>
			static To eval (T_&& self) {
				return static_cast<To> (std::forward<T_> (self));
			}
		};
	};

	/**
	 Gives access to the underlying typed C++ object from an Object value. To has to be a reference (const &, & or
	 &&). Will throw an exception if either self doesn't hold an object of the specified type, or if To is non
	 const and mutable access to the object is unsafe (i.e. self is shared, weak, interned)
	 */
	template<typename To, typename T>
	To obj_cast (T&& self) {
		return Trait_Object<std::decay_t<T>>::template obj_cast<To>::eval (std::forward<T> (self));
	};
}
