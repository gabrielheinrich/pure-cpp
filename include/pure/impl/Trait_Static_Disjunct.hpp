#pragma once

#include <pure/symbolic_sets.hpp>
#include <pure/support/type_utilities.hpp>

namespace pure {
	template<auto A, auto B>
	struct Trait_Static_Disjunct<Category_t < A>, Category_t <B>> : Trait_Definition {
	static constexpr bool eval () { return A != B; }
};

template<>
struct definitely_disjunct_t<None_t, None_t> {
	static constexpr bool eval () { return true; }
};

template<typename B>
struct Trait_Static_Disjunct<None_t, B> : Trait_Definition {
	static constexpr bool eval () { return true; }
};

template< typename A_First, typename... A_Rest, typename B >
struct Trait_Static_Disjunct< Union_t < A_First, A_Rest ... >, B > : Trait_Definition {
	static constexpr bool value = definitely_disjunct_t<A_First,B>::eval() ? definitely_disjunct_t<Union_t <A_Rest...>, B>::eval() : false;
	static constexpr bool eval () {
		return value;
	}
};

template<typename B>
struct Trait_Static_Disjunct<Any_t, B> : Trait_Definition {
	static constexpr bool eval () { return false; }
};

template<>
struct Trait_Static_Disjunct<Any_t, None_t> : Trait_Definition {
	static constexpr bool eval () { return true; }
};

template<intmax_t A_Min, intmax_t A_Max, typename B>
struct Trait_Static_Disjunct<Ints_t < A_Min, A_Max>, B> : Trait_Definition {
static constexpr bool eval () { return definitely_disjunct_t<Int_t, B>::eval (); }
};

template<intmax_t A_Min, intmax_t A_Max, intmax_t B_Min, intmax_t B_Max>
struct Trait_Static_Disjunct<Ints_t < A_Min, A_Max>, Ints_t <B_Min, B_Max>> : Trait_Definition {
static constexpr bool eval () { return A_Max < B_Min || A_Min > B_Max; }
};

namespace detail {
	template<typename A, typename B>
	struct type_lists_definitely_disjunct {
		static constexpr bool eval () {
			static_assert (A::length == B::length);
			if constexpr (A::empty) return false;
			else if constexpr (definitely_disjunct_t<typename A::first, typename B::first>::eval ()) return true;
			else return type_lists_definitely_disjunct<typename A::rest, typename B::rest>::eval ();
		}
	};

	template<typename List, typename B>
	struct type_list_definitely_disjunct {
		static constexpr bool eval () {
			if constexpr (List::empty) return false;
			else if constexpr (definitely_disjunct < typename List::first, B >) return true;
			else return type_list_definitely_disjunct<typename List::rest, B>::eval ();
		}
	};
}

template<typename... A_Args, typename B>
struct Trait_Static_Disjunct<Function_t < A_Args...>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct_t < Any_Function_t , B >::eval();
}
};

template<typename... A_Args, typename... B_Args>
struct Trait_Static_Disjunct<Function_t < A_Args...>, Function_t<B_Args...>> : Trait_Definition {
static constexpr bool eval () {
	if constexpr (sizeof... (A_Args) != sizeof... (B_Args))
		return true;
	else
		return detail::type_lists_definitely_disjunct<detail::type_list<A_Args...>, detail::type_list<B_Args...>>::eval ();
}
};

template<typename Elements, typename B>
struct Trait_Static_Disjunct<Set_t < Elements>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct<Any_Set_t, B>;
}
};

template<typename A_Elements, typename B_Elements>
struct Trait_Static_Disjunct<Set_t < A_Elements>, Set_t <B_Elements>> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct<A_Elements, B_Elements>;
}
};

template<typename Elements, typename B>
struct Trait_Static_Disjunct<Vector_t < Elements>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct_t<Any_Vector_t, B>::eval ();
}
};

template<typename A_Elements, typename B_Elements>
struct Trait_Static_Disjunct<Vector_t < A_Elements>, Vector_t <B_Elements>> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct<A_Elements, B_Elements>;
}
};

template<typename... Elements, typename B>
struct Trait_Static_Disjunct<Tuple_t < Elements...>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct<Any_Vector_t, B>;
}
};

template<typename... A_Elements, typename... B_Elements>
struct Trait_Static_Disjunct<Tuple_t < A_Elements...>, Tuple_t<B_Elements...>> : Trait_Definition {
static constexpr bool eval () {
	if constexpr (sizeof... (A_Elements) != sizeof... (B_Elements)) return false;
	else return detail::type_lists_definitely_disjunct<detail::type_list<A_Elements...>, detail::type_list<B_Elements...>>::eval ();
}
};

template<typename... A_Elements, typename B_Elements>
struct Trait_Static_Disjunct<Tuple_t < A_Elements...>, Vector_t <B_Elements>> : Trait_Definition {
static constexpr bool eval () {
	return detail::type_list_definitely_disjunct<detail::type_list<A_Elements...>, B_Elements>::eval ();
}
};

template<typename A_Elements, typename... B_Elements>
struct Trait_Static_Disjunct<Vector_t < A_Elements>, Tuple_t<B_Elements...>> : Trait_Definition {
static constexpr bool eval () {
	return detail::type_list_definitely_disjunct<detail::type_list<B_Elements...>, A_Elements>::eval ();
}
};

template<typename I, typename B>
struct Trait_Static_Disjunct<Id_t < I>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct_t<String_t, B>::eval ();
}
};

template<typename A_id, typename B_id>
struct Trait_Static_Disjunct<Id_t < A_id>, Id_t <B_id>> : Trait_Definition {
static constexpr bool eval () {
	return std::is_same_v<A_id, B_id>;
}
};

namespace detail {
	template<typename A, typename B>
	struct id_value_pair_list_definitely_disjunct {
		static constexpr bool eval () {
			if constexpr (A::empty) return false;
			else {
				using First = typename A::first;

				if constexpr (std::is_same_v<typename First::first, typename B::first>) {
					if constexpr (definitely_disjunct < typename First::second, typename B::second >) return true;
					else return false;
				}
				else return id_value_pair_list_definitely_disjunct<typename A::rest, B>::eval ();
			}
		}
	};

	template<typename A, typename B>
	struct id_value_pair_lists_definitely_disjunct {
		static constexpr bool eval () {
			if constexpr (A::empty) return false;
			else if constexpr (id_value_pair_list_definitely_disjunct<B, typename A::first>::eval ()) return true;
			else return id_value_pair_lists_definitely_disjunct<typename A::rest, B>::eval ();
		}
	};
}

template<typename... Id_Value_Pairs, typename B>
struct Trait_Static_Disjunct<Record_t < Id_Value_Pairs...>, B> : Trait_Definition {
static constexpr bool eval () {
	return definitely_disjunct < Function_t < String_t, Any_t >, B >;
}
};

template<typename... A_Id_Value_Pairs, typename... B_Id_Value_Pairs>
struct Trait_Static_Disjunct<Record_t < A_Id_Value_Pairs...>, Record_t<B_Id_Value_Pairs...>> : Trait_Definition {
static constexpr bool eval () {
	using namespace detail;
	if constexpr (sizeof... (A_Id_Value_Pairs) != sizeof... (B_Id_Value_Pairs)) return false;
	else
		return detail::id_value_pair_lists_definitely_disjunct<
				typename pair_type_list<type_list < A_Id_Value_Pairs...>>::value, typename pair_type_list<
				type_list < B_Id_Value_Pairs...>>
	::value > ::eval ();
}
};
}