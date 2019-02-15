#pragma once

#include <pure/traits.hpp>

namespace pure {
	template<typename T>
	struct Trait_Error<T, Type_Class::Exception> : Trait_Definition {
		static const char* error_message (const T& self) {
			return self.what ();
		}
	};

	template<typename T>
	struct Trait_Error<T, Type_Class::Var> : Trait_Definition {
		static const char* error_message (const T& self) {
			switch (self.tag ()) {
				case Var_Tag_Pointer : self->error_message ();
				default : throw operation_not_supported ();
			}
		}
	};
}