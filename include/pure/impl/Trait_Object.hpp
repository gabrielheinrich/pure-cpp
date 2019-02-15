#pragma once

#include <pure/traits.hpp>

namespace pure {
	template<typename T>
	struct Trait_Object<T, Type_Class::Var> : Trait_Definition {
		template<typename To>
		struct obj_cast {
			template<typename T_>
			static To eval (T_&& self) {
				static_assert (std::is_reference_v<To>, "obj_cast only works with references");
				constexpr bool To_is_const = std::is_const_v<std::remove_reference_t<To>>;
				constexpr bool Self_is_const = std::is_const_v<std::remove_reference_t<T_&&>>;

				static_assert (To_is_const || !Self_is_const);
				if (!To_is_const) {
					switch (self.tag ()) {
						case Var::Tag::Moveable :
						case Var::Tag::Unique : break;
						case Var::Tag::Shared : if (detail::ref_count_is_unique (*self)) break;
						default : throw operation_not_supported ();
					}
				}
				switch (self.tag ()) {
					case Var_Tag_Pointer : {
						if constexpr (std::is_base_of_v<Interface::Value, std::decay_t<To>>) {
							auto obj = dynamic_cast<std::decay_t<To>*> (self.operator-> ());
							if (obj) return *obj; else throw operation_not_supported ();
						}
						else {
							auto obj = dynamic_cast<Boxed <std::decay_t<To>>*> (self.operator-> ());
							if (obj) return obj->self; else throw operation_not_supported ();
						}
					}
					default: throw operation_not_supported ();
				}

			}
		};
	};
}