#pragma once

#include <pure/traits.hpp>
#include <cstring>

namespace pure {
	template<typename T>
	struct Trait_CString<T, Type_Class::CString> : Trait_Definition {
		static const char* raw_cstring (const T& self) { return self; }
		static intptr_t raw_cstring_length (const T& self) { return std::strlen (self); }
	};

	template<typename T>
	struct Trait_CString<T, Type_Class::Identifier> : Trait_Definition {
		static const char* raw_cstring (const T& self) { return self.string; }
		static intptr_t raw_cstring_length (const T& self) { return self.length; }
	};

	template<typename T>
	struct Trait_CString<T, Type_Class::Var> : Trait_Definition {
		static const char* raw_cstring (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return self.get_cstring ();
				case Var_Tag_Pointer : return self->cstring ();
				default : throw operation_not_supported ();
			}
		}
		static intptr_t raw_cstring_length (const T& self) {
			switch (self.tag ()) {
				case Var::Tag::String : return std::strlen (self.get_cstring ());
				case Var_Tag_Pointer : return self->cstring_length ();
				default : throw operation_not_supported ();
			}
		}
	};
}