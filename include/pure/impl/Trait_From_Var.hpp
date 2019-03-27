#pragma once

#include <pure/traits.hpp>

namespace pure {
	template<typename T>
	struct Trait_From_Var<T, Type_Class::Bool> : Trait_Definition {
		static constexpr bool to_ref_implemented = false;

		template<typename From>
		struct from {
			static T convert (const From& self) {
				switch (self.tag ()) {
					case Var::Tag::Nil : return (T)false;
					case Var::Tag::False : return (T)false;
					case Var::Tag::Int : return (T)self.get_int ();
					case Var::Tag::Int64 : return (T)self.get_int64 ();
					case Var::Tag::Double : return (T)self.get_double ();
					case Var::Tag::Char : return (T)self.get_char ();
					default : return true;
				}
			}
		};
	};

	template<typename T>
	struct Trait_From_Var<T, Type_Class::Int> : Trait_Definition {
		static constexpr bool to_ref_implemented = false;

		template<typename From>
		struct from {
			static T convert (const From& self) {
				switch (self.tag ()) {
					case Var::Tag::Nil : return (T)0;
					case Var::Tag::False : return (T)0;
					case Var::Tag::True : return (T)1;
					case Var::Tag::Int : return (T)self.get_int ();
					case Var::Tag::Int64 : return (T)self.get_int64 ();
					case Var::Tag::Double : return (T)self.get_double ();
					case Var::Tag::Char : return (T)self.get_char ();
					default : throw operation_not_supported ();
				}
			}
		};
	};

	template<typename T>
	struct Trait_From_Var<T, Type_Class::Double> : Trait_Definition {
		static constexpr bool to_ref_implemented = false;

		template<typename From>
		struct from {
			static T convert (const From& self) {
				switch (self.tag ()) {
					case Var::Tag::Nil : return (T)0;
					case Var::Tag::False : return (T)0;
					case Var::Tag::True : return (T)1;
					case Var::Tag::Int : return (T)self.get_int ();
					case Var::Tag::Int64 : return (T)self.get_int64 ();
					case Var::Tag::Double : return (T)self.get_double ();
					case Var::Tag::Char : return (T)self.get_char ();
					default : throw operation_not_supported ();
				}
			}
		};
	};

	template<typename T>
	struct Trait_From_Var<T, Type_Class::Character> : Trait_Definition {
		static constexpr bool to_ref_implemented = false;

		template<typename From>
		struct from {
			static T convert (const From& self) {
				switch (self.tag ()) {
					case Var::Tag::Nil : return (T)0;
					case Var::Tag::False : return (T)0;
					case Var::Tag::True : return (T)1;
					case Var::Tag::Int : return (T)self.get_int ();
					case Var::Tag::Int64 : return (T)self.get_int64 ();
					case Var::Tag::Double : return (T)self.get_double ();
					case Var::Tag::Char : return (T)self.get_char ();
					default : throw operation_not_supported ();
				}
			}
		};
	};

	template<>
	struct Trait_From_Var<const char*, Type_Class::CString> : Trait_Definition {
		static constexpr bool to_ref_implemented = false;

		template<typename From>
		struct from {
			static const char* convert (const From& self) {
				switch (self.tag ()) {
					case Var::Tag::String : return self.get_cstring ();
					case Var_Tag_Pointer : return self->cstring ();
					default : throw operation_not_supported ();
				}
			}
		};
	};
}