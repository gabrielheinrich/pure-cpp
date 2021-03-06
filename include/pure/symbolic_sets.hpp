#pragma once

#include <pure/support/type_utilities.hpp>
#include <pure/type_class.hpp>
#include <pure/traits.hpp>
#include <pure/support/identifier.hpp>

namespace pure {
	template<typename T>
	struct deref_symbolic_set_t { using value = std::decay_t<T>; };

	template<typename T>
	using deref_symbolic_set = typename deref_symbolic_set_t<T>::value;

	enum class Category_Id : int {
		Nil,
		False,
		True,
		Int,
		Double,
		Character,
		String,
		Function,
		Set,
		Vector,
		Error,
		Object
	};

	template<auto Id>
	struct Category_t : implements<Type_Class::Symbolic_Set> {
		static constexpr int id = static_cast<int>(Id);

		template<typename T>
		bool operator() (const T& other) const { return pure::category_id (other) == id; }
	};

	template<>
	struct Category_t<Category_Id::Nil> : implements<Type_Class::Symbolic_Set> {
		static constexpr int id = static_cast<int>(Category_Id::Nil);

		template<typename T>
		bool operator() (const T& other) const { return Var::tag (other) == Var::Tag::Nil; }
	};

	using Nil_t = Category_t<Category_Id::Nil>;
	/**
	 constexpr set containing the value nullptr
	 */
	constexpr auto Nil = static_instance<Nil_t>::instance;

	using False_t = Category_t<Category_Id::False>;
	/**
	 constexpr set containing the value false
	 */
	constexpr auto False = static_instance<False_t>::instance;

	using True_t = Category_t<Category_Id::True>;
	/**
	 constexpr set containing the value true.
	 */
	constexpr auto True = static_instance<True_t>::instance;

	using Int_t = Category_t<Category_Id::Int>;
	/**
	 constexpr set of all integers
	 */
	constexpr auto Int = static_instance<Int_t>::instance;

	using Double_t = Category_t<Category_Id::Double>;
	/**
	 constexpr set of all double-percision floating point numbers
	 */
	constexpr auto Double = static_instance<Double_t>::instance;

	using Character_t = Category_t<Category_Id::Character>;
	/**
	 constexpr set of all unicode code points
	 */
	constexpr auto Character = static_instance<Character_t>::instance;

	using String_t = Category_t<Category_Id::String>;
	/**
	 constexpr set of all utf8 strings
	 */
	constexpr auto String = static_instance<String_t>::instance;

	using Any_Function_t = Category_t<Category_Id::Function>;
	/**
	 constexpr set of all values in the category Function
	 */
	constexpr auto Any_Function = static_instance<Any_Function_t>::instance;

	using Any_Set_t = Category_t<Category_Id::Set>;
	/**
	 constexpr set of all values in the category Set
	 */
	constexpr auto Any_Set = static_instance<Any_Set_t>::instance;

	using Any_Vector_t = Category_t<Category_Id::Vector>;
	/**
	 constexpr set of all values in the category Vector
	 */
	constexpr auto Any_Vector = static_instance<Any_Vector_t>::instance;

	using Error_t = Category_t<Category_Id::Error>;
	/**
	 constexpr set of all values in the category Error
	 */
	constexpr auto Error = static_instance<Error_t>::instance;

	using Object_t = Category_t<Category_Id::Object>;
	/**
	 constexpr set of all values in the category Object
	 */
	constexpr auto Object = static_instance<Object_t>::instance;

	template<typename... Sets>
	struct Union_t : implements<Type_Class::Symbolic_Set> {
		template<typename List, typename T>
		static constexpr bool eval (const T& other) {
			if constexpr (List::empty) return false;
			else {
				if (typename List::first {} (other)) return true;
				else return eval<typename List::rest> (other);
			}
		}

		template<typename T>
		bool operator() (const T& other) const {
			return eval<detail::type_list<Sets...>> (other);
		}
	};

	/**
	 constexpr set that is the union of var number of constexpr sets
	 */
	template<const auto& ... Sets>
	constexpr auto Union = static_instance<Union_t<deref_symbolic_set<decltype (Sets)>...>>::instance;

	using None_t = Union_t<>;
	/**
	 constexpr set that is the empty set
	 */
	constexpr auto None = static_instance<None_t>::instance;

	using Bool_t = Union_t<False_t, True_t>;
	/**
	 constexpr set that contains true and false
	 */
	constexpr auto Bool = static_instance<Bool_t>::instance;

	struct Any_t : implements<Type_Class::Symbolic_Set> {
		template<typename T>
		bool operator() (const T& other) const noexcept { return true; }
	};

	/**
	 constexpr set that contains all values.
	 */
	constexpr auto Any = static_instance<Any_t>::instance;

	template<intmax_t Min, intmax_t Max>
	struct Ints_t : implements<Type_Class::Symbolic_Set> {
		constexpr Ints_t() {};
		template<typename T>
		bool operator() (const T& other) const {
			if constexpr (std::is_convertible_v<T, intmax_t>) {
				if (pure::category_id (other) == Int.id) {
					auto value = static_cast<intmax_t> (other);
					return value >= Min && value <= Max;
				}
				else return false;

			}
			else return false;
		}
	};

	/**
	 constexpr set that contains all integers [Min, ..., Max]
	 */
	template<intmax_t Min, intmax_t Max>
	constexpr auto Ints = static_instance<Ints_t<Min, Max>>::instance;

	using Byte_t = Ints_t<0, UINT8_MAX>;
	/**
	 constexpr set that contains all integers [0, ..., 255]
	 */
	constexpr auto Byte = static_instance<Byte_t>::instance;

	using Int8_t = Ints_t<INT8_MIN, INT8_MAX>;
	/**
	 constexpr set that contains all integers that fit in an int8_t
	 */
	constexpr auto Int8 = static_instance<Int8_t>::instance;

	using Int16_t = Ints_t<INT16_MIN, INT16_MAX>;
	/**
	 constexpr set that contains all integers that fit in an int16_t
	 */
	constexpr auto Int16 = static_instance<Int16_t>::instance;

	using Int32_t = Ints_t<INT32_MIN, INT32_MAX>;
	/**
	 constexpr set that contains all integers that fit in an int32_t
	 */
	constexpr auto Int32 = static_instance<Int32_t>::instance;

	using Int64_t = Ints_t<INT64_MIN, INT64_MAX>;
	/**
	 constexpr set that contains all integers that fit in an int64_t
	 */
	constexpr auto Int64 = static_instance<Int64_t>::instance;

	template<typename... Args>
	struct Function_t : implements<Type_Class::Symbolic_Set> {
		template<typename T>
		bool operator() (const T& other) const {
			if (category_id (other) == Any_Function.id) {
				if (Variadic (other)) return false;
				if (arity (other) == sizeof... (Args) - 1)
					return true;
				return false;
			}
			else
				return false;
		}
	};

	/**
	 constexpr set of functions, with some restriction on the domain and co-domain.
	 */
	template<const auto& ... Args>
	constexpr auto Function = static_instance<Function_t<deref_symbolic_set<decltype (Args)>...>>::instance;

	template<typename Elements>
	struct Set_t : implements<Type_Class::Symbolic_Set> {
		template<typename T>
		bool operator() (const T& other) const {
			if (category_id (other) == Any_Set.id) {
				if (arity (other) != 1) return false;
				if constexpr (std::is_same_v<Elements, Any_t>) return true;
				else if constexpr (Trait_Enumerable<T>::implemented) {
					for (auto e = enumerate (other); !e.empty (); e.next ()) {
						if (Elements {} (e.read ()) == false) return false;
					}
					return true;
				}
				else throw operation_not_supported ();
			}
			else
				return false;
		}
	};

	/**
	 constexpr set of sets, with some restriction on the contained elements.
	 */
	template<const auto&... Elements>
	constexpr auto Set = static_instance<Set_t<deref_symbolic_set<decltype (Elements)>...>>::instance;

	template<typename Elements>
	struct Vector_t : implements<Type_Class::Symbolic_Set> {
		constexpr Vector_t() {};

		template<typename T>
		constexpr bool operator() (const T& other) const {
			if (category_id (other) == Any_Vector.id) {
				if constexpr (Trait_Enumerable<T>::implemented) {
					for (auto e = enumerate (other); !e.empty (); e.next ()) {
						if (Elements {} (e.read ()) == false) return false;
					}
					return true;
				}
				else throw operation_not_supported ();
			}
			else
				return false;
		}
	};

	/**
	 constexpr set of vectors, with some restriction on the elements in the vector.
	 */
	template<const auto&... Elements>
	constexpr auto Vector = static_instance<Vector_t<deref_symbolic_set<decltype (Elements)>...>>::instance;

	template<typename... Elements>
	struct Tuple_t : implements<Type_Class::Symbolic_Set> {
		template<typename List, typename Enumerator>
		bool eval (Enumerator& e) const {
			if constexpr (List::empty) return e.empty ();
			else {
				if (typename List::first {} (e.read ()) == false) return false;
				e.next ();
				return eval<typename List::rest> (e);
			}
		}

		template<typename T>
		bool operator() (const T& other) const {
			if (category_id (other) == Any_Vector.id) {
				if constexpr (Trait_Enumerable<T>::implemented) {
					if (count (other) != sizeof... (Elements)) return false;
					auto e = enumerate (other);
					return eval<detail::type_list<Elements...>> (e);
				}
				else throw operation_not_supported ();
			}
			else
				return false;
		}
	};

	/**
	 constexpr set of vectors of a specific size and with some restrictions on the individual elements within it.
	 */
	template<const auto& ... Elements>
	constexpr auto Tuple = static_instance<Tuple_t<deref_symbolic_set<decltype (Elements)>...>>::instance;

	template<typename I>
	struct Id_t : implements<Type_Class::Symbolic_Set> {
		template<typename T>
		bool operator() (const T& other) const {
			return equal (I{}, other);
		}
	};

	template<typename... Id_Value_Pairs>
	struct Record_t : implements<Type_Class::Symbolic_Set> {
		template<typename List, typename T>
		static bool eval (const T& other) {
			if constexpr (List::empty) return true;
			else {
				using First = typename List::first;
				if (typename First::second {} (apply (other, typename First::first {})) == false)
					return false;
				return eval<typename List::rest> (other);
			}
		}

		template<typename T>
		bool operator() (const T& other) const {
			if (category_id (other) == Any_Function.id) {
				return eval<typename detail::pair_type_list<detail::type_list<Id_Value_Pairs...>>::value> (other);
			}
			else
				return false;
		}
	};

	/**
	 constexpr set of enumerable functions from keys to values, in which each key is a specific string and each value
	 can be restricted to a domain.
	 */
	template<const auto& ... Id_Values>
	constexpr auto Record = static_instance<Record_t<deref_symbolic_set<decltype (Id_Values)>...>>::instance;
}
