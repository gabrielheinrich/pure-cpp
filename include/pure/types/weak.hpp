#pragma once

#include <pure/types/some.hpp>

namespace pure {
	/**
	 Weak unsafe pointer to a value. The lifetime of the value has to be managed by some other value holder.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Nil Either never_nil or maybe_nil to specify whether nullptr should be included in the domain of this type.
	 */
	template<typename O = Interface::Value, typename Nil = never_nil>
	struct weak : some<O, Nil> {
		using object_type = O;
		static constexpr bool definitely_const_pointer = true;

		weak (O* other) { this->init_ptr (Var::Tag::Weak, other); }

		template<typename T>
		weak (const T& other) {
			static_assert (!definitely_disjunct_t<typename weak::domain_t, pure::domain_t<T>>::eval ());
			switch (Var::tag_for<object_type> (other)) {
				case Var::Tag::Nil : {
					if constexpr (Nil::is_never_nil)
						throw operation_not_supported ();
					else
						this->init_nil ();
				}
					return;
				case Var_Tag_Pointer : {
					this->init_ptr (Var::Tag::Weak, Var::obj (other));
					return;
				}
				default : throw operation_not_supported ();
			}
		}
		weak (const weak& other) : weak (static_cast<const weak&&> (other)) {}

		~weak () {
			this->init_nil ();
		}

		Var::Tag tag () const noexcept {
			if constexpr (Nil::is_never_nil) return Var::Tag::Weak;
			else return this->value ? Var::Tag::Weak : Var::Tag::Nil;
		}
	};
}
