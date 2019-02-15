#pragma once

#include <pure/traits.hpp>
#include <pure/symbolic_sets.hpp>
#include <pure/types/restrict.hpp>

namespace pure {
	template<typename T>
	struct Trait_Value<T, Type_Class::Nil> : Trait_Definition {
		static constexpr int category_id (const T&) { return Nil.id; }
		using domain_t = Nil_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Bool> : Trait_Definition {
		static constexpr int category_id (const T& self) { return self ? True.id : False.id; }
		using domain_t = Bool_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Int> : Trait_Definition {
		static constexpr int category_id (const T&) { return Int.id; }
		using domain_t = Int_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Double> : Trait_Definition {
		static constexpr int category_id (const T&) { return Double.id; }
		using domain_t = Double_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Character> : Trait_Definition {
		static constexpr int category_id (const T&) { return Character.id; }
		using domain_t = Character_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::CString> : Trait_Definition {
		static constexpr int category_id (const T&) { return String.id; }
		using domain_t = String_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Identifier> : Trait_Definition {
		static constexpr int category_id (const T&) { return String.id; }
		using domain_t = String_t;
	};

	template<typename T>
	struct Trait_Value<T, Type_Class::Var> : Trait_Definition {
		static int category_id (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::Nil : return Nil.id;
				case Var::Tag::False : return False.id;
				case Var::Tag::True : return True.id;
				case Var::Tag::Int :
				case Var::Tag::Int64 : return Int.id;
				case Var::Tag::Double : return Double.id;
				case Var::Tag::Char : return Character.id;
				case Var::Tag::String : return String.id;
				case Var_Tag_Pointer : return self->category_id ();
				default : assert (0);
					return Nil.id;
			}
		}

		using domain_t = typename T::domain_t;
	};

	template<typename T, typename R, typename... Args>
	struct Trait_Value<T, Type_Class::Function < R, Args...>> : Trait_Definition {
	static constexpr bool is_set = std::is_same_v<R, bool>;

	static int category_id (const T&) {
		if constexpr (is_set)
			return Any_Set.id;
		else
			return Any_Function.id;
	}

	using domain_t = std::conditional_t<is_set, Any_Set_t, Any_Function_t>;
};

template<typename T, typename R, intptr_t Arity, bool is_Variadic>
struct Trait_Value<T, Type_Class::Generic_Function < R, Arity, is_Variadic>> : Trait_Definition {
static constexpr bool is_set = std::is_same_v<R, bool>;

static int category_id (const T&) {
	if constexpr (is_set)
		return Any_Set.id;
	else
		return Any_Function.id;
}

using domain_t = std::conditional_t<is_set, Any_Set_t, Any_Function_t>;
};

template<typename T>
struct Trait_Value<T, Type_Class::Symbolic_Set> : Trait_Definition {
	static int category_id (const T&) { return Any_Set.id; }
	using domain_t = Any_Set_t;
};

template<typename T, typename... Elements>
struct Trait_Value<T, Type_Class::Tuple < Elements...>> : Trait_Definition {
static int category_id (const T&) { return Any_Vector.id; }
using domain_t = Any_Vector_t;
};

template<typename T, typename Capabilities>
struct Trait_Value<T, Type_Class::Iterable < Capabilities>> : Trait_Definition {
static int category_id (const T&) { return Any_Vector.id; }
using domain_t = Any_Vector_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::Sequence> : Trait_Definition {
	static int category_id (const T&) { return Any_Vector.id; }
	using domain_t = Any_Vector_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::Exception> : Trait_Definition {
	static int category_id (const T&) { return Error.id; }
	using domain_t = Error_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::Std_Stream> : Trait_Definition {
	static int category_id (const T&) { return Object.id; }
	using domain_t = Object_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::File_Stream> : Trait_Definition {
	static int category_id (const T&) { return Object.id; }
	using domain_t = Object_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::Stream> : Trait_Definition {
	static int category_id (const T&) { return Object.id; }
	using domain_t = Object_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::External> : Trait_Definition {
	static int category_id (const T&) { return Object.id; }
	using domain_t = Object_t;
};

template<typename T>
struct Trait_Value<T, Type_Class::Record> : Trait_Definition {
	static int category_id (const T&) { return Any_Function.id; }
	using domain_t = Any_Function_t;
};

template<typename T, typename Domain>
struct Trait_Value<restricted < T, Domain>, Type_Class::Restricted> : Trait_Definition {
static constexpr int category_id (const restricted <T, Domain>& value) {
	return category_id (static_cast<const T&> (value));
}
using domain_t = Domain;
};

}
