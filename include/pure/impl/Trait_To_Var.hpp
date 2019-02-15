#pragma once

#include <cstring>
#include <pure/traits.hpp>
#include <pure/object/interface.hpp>
#include <pure/object/boxed.hpp>
#include <pure/object/basic_string.hpp>

namespace pure {
	template<typename T>
	struct Trait_To_Var_Default : Trait_Definition {
		using object_type = Interface::Value;

		static constexpr Var::Tag tag (const T& self) { return Var::Tag::External; }

		static constexpr object_type* obj (const T&) { return nullptr; }
		static constexpr object_type* release (T&) { return nullptr; }

		static bool constexpr has_interned = false;

		static intptr_t get_int (const T&) { return 0; }
		static int64_t get_int64 (const T&) { return 0; }
		static double get_double (const T&) { return 0.0; }
		static char32_t get_char (const T&) { return 0; }
		static const char* get_cstring (const T&) { return nullptr; }
		static intptr_t get_cstring_length (const T&) { return 0; }

		static Interface::Value* clone (const T&) { throw operation_not_supported (); }
		static intptr_t clone_bytes_needed (const T&) { throw operation_not_supported (); }
		static Interface::Value* clone_placement (const T&, void*, intptr_t) { throw operation_not_supported (); }
	};

	template<typename T, typename Clone_Type>
	struct Trait_To_Var_Default_Clone : Trait_To_Var_Default<T> {
		template<typename U>
		static Interface::Value* clone (U&& self) { return create<Clone_Type> (std::forward<U> (self)); }

		static intptr_t clone_bytes_needed (const T& self) { return create_num_bytes_needed<Clone_Type> (self); }

		template<typename U>
		static Interface::Value* clone_placement (U&& self, void* memory, intptr_t num_bytes) {
			return create_placement<Clone_Type> (memory, create_capacity_needed<Clone_Type> (self),
												 std::forward<U> (self));
		}
	};

	template<typename T>
	struct Trait_To_Var<T, Type_Class::Nil> : Trait_To_Var_Default<T> {
		static constexpr Var::Tag tag (const T&) { return Var::Tag::Nil; }
	};

	template<typename T>
	struct Trait_To_Var<T, Type_Class::Bool> : Trait_To_Var_Default_Clone<T, Boxed < bool>> {
	static constexpr Var::Tag tag (const T& self) { return self ? Var::Tag::True : Var::Tag::False; }
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Int> : Trait_To_Var_Default_Clone<T, Boxed < int64_t>> {
static constexpr Var::Tag tag (const T& self) {
	return any::int_in_range (self) ? Var::Tag::Int : Var::Tag::Int64;
}

static intptr_t get_int (const T& self) { return (intptr_t)self; }
static int64_t get_int64 (const T& self) { return (int64_t)self; }
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Double> : Trait_To_Var_Default_Clone<T, Boxed < double>> {
static constexpr Var::Tag tag (const T&) { return Var::Tag::Double; }
static double get_double (const T& self) { return self; }
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Character> : Trait_To_Var_Default_Clone<T, Boxed < char32_t>> {
static constexpr Var::Tag tag (const T& self) { return Var::Tag::Char; }
static char32_t get_char (const T& self) { return self; }
};

template<typename T>
struct Trait_To_Var<T, Type_Class::CString> : Trait_To_Var_Default_Clone<T, Basic::String> {
	static constexpr Var::Tag tag (const T& self) { return Var::Tag::String; }
	static const char* get_cstring (const T& self) { return self; }
	static intptr_t get_cstring_length (const T& self) { return std::strlen (self); }
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Identifier> : Trait_To_Var_Default_Clone<T, Basic::String> {
	static constexpr Var::Tag tag (const T& self) { return Var::Tag::String; }
	static const char* get_cstring (const T& self) { return self.string; }
	static intptr_t get_cstring_length (const T& self) { return self.length; }

	static constexpr bool has_interned = true;
	static Interface::Value* obj (const T&) {
		if constexpr (has_interned)
			return (Interface::Value * ) (&static_instance < Boxed < T >> ::instance);
		else return nullptr;
	}
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Var> : Trait_Definition {
	using object_type = typename T::object_type;

	static constexpr Var::Tag tag (const T& self) { return self.tag (); }
	static object_type* obj (const T& self) { return self.operator-> (); }
	static object_type* release (T& self) { return self.release (); }

	static constexpr bool has_interned = false;
	static intptr_t get_int (const T& self) { return self.get_int (); }
	static int64_t get_int64 (const T& self) { return self.get_int64 (); }
	static char32_t get_char (const T& self) { return self.get_char (); }
	static double get_double (const T& self) { return self.get_double (); }
	static const char* get_cstring (const T& self) { return self.get_cstring (); }
	static intptr_t get_cstring_length (const T& self) { return std::strlen (self.get_cstring ()); }

	template<typename TT>
	static Interface::Value* clone (TT&& self) {
		switch (self.tag ()) {
			case Var::Tag::Unique : if constexpr (detail::is_moveable < TT &&>)
				return self.release ();
			case Var::Tag::Moveable : if constexpr (detail::is_moveable < TT &&>)
				return std::move (*self).clone ();
			case Var::Tag::Shared :
			case Var::Tag::Weak :
			case Var::Tag::Interned : return self->clone ();
			case Var::Tag::Int : return Var::clone (self.get_int ());
			case Var::Tag::Int64 : return Var::clone (self.get_int64 ());
			case Var::Tag::Double : return Var::clone (self.get_double ());
			case Var::Tag::Char : return Var::clone (self.get_char ());
			case Var::Tag::String : return Var::clone (self.get_cstring ());
			case Var::Tag::True : return Var::clone (true);
			case Var::Tag::False : return Var::clone (false);
			default : throw operation_not_supported ();
		}
	}

	static intptr_t clone_bytes_needed (const T& self) {
		switch (self.tag ()) {
			case Var::Tag::False :
			case Var::Tag::True : return Var::clone_bytes_needed (true);
			case Var::Tag::Int : return Var::clone_bytes_needed (intptr_t (0));
			case Var::Tag::Int64 : return Var::clone_bytes_needed (int64_t (0));
			case Var::Tag::Double : return Var::clone_bytes_needed (double (0));
			case Var::Tag::Char : return Var::clone_bytes_needed (char32_t (0));
			case Var::Tag::String : return Var::clone_bytes_needed (self.get_cstring ());
			case Var_Tag_Pointer : return self->clone_bytes_needed ();
			default : throw operation_not_supported ();
		}
	}

	template<typename TT>
	static Interface::Value* clone_placement (TT&& self, void* memory, intptr_t num_bytes) {
		switch (self.tag ()) {
			case Var::Tag::Unique :
			case Var::Tag::Moveable : if constexpr (detail::is_moveable < TT &&>)
				return std::move (*self).clone_placement (memory, num_bytes);
			case Var::Tag::Shared :
			case Var::Tag::Weak :
			case Var::Tag::Interned : return self->clone_placement (memory, num_bytes);
			case Var::Tag::Int : return Var::clone_placement (self.get_int (), memory, num_bytes);
			case Var::Tag::Int64 : return Var::clone_placement (self.get_int64 (), memory, num_bytes);
			case Var::Tag::Double : return Var::clone_placement (self.get_double (), memory, num_bytes);
			case Var::Tag::Char : return Var::clone_placement (self.get_char (), memory, num_bytes);
			case Var::Tag::String : return Var::clone_placement (self.get_cstring (), memory, num_bytes);
			case Var::Tag::True : return Var::clone_placement (true, memory, num_bytes);
			case Var::Tag::False : return Var::clone_placement (false, memory, num_bytes);
			default : throw operation_not_supported ();
		}
	}
};

template<typename T, typename R, typename... Args>
struct Trait_To_Var<T, Type_Class::Function < R, Args...>> : Trait_To_Var_Default_Clone <T, Boxed<T>> {
};

template<typename T, typename R, intptr_t Arity, bool is_Variadic>
struct Trait_To_Var<T,
		Type_Class::Generic_Function < R, Arity, is_Variadic>> : Trait_To_Var_Default_Clone <T, Boxed<T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Symbolic_Set> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T, typename... Elements>
struct Trait_To_Var<T, Type_Class::Tuple < Elements...>> : Trait_To_Var_Default_Clone <T, Boxed<T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Record> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T, typename Capabilities>
struct Trait_To_Var<T, Type_Class::Iterable < Capabilities>> : Trait_To_Var_Default_Clone <T, Boxed<T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Sequence> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Exception> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::File_Stream> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::Stream> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};

template<typename T>
struct Trait_To_Var<T, Type_Class::External> : Trait_To_Var_Default_Clone<T, Boxed < T>> {
};
}