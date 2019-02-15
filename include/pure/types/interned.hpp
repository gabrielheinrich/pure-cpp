#pragma once

#include <pure/types/some.hpp>

namespace pure {
	struct intern_tag {};
	constexpr intern_tag intern {};

	/**
	 Stable pointer to a value. The lifetime of the value is handled by some statically initialized context.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Nil Either never_nil or maybe_nil to specify whether nullptr should be included in the domain of this type.
	 */
	template<typename O = Interface::Value, typename Nil = never_nil>
	struct interned : some<O, Nil> {
		using object_type = O;
		static constexpr bool definitely_const_pointer = true;

		interned (intern_tag, const O* ptr) { this->init_ptr (Var::Tag::Interned, const_cast<O*>(ptr)); }

		template<typename T>
		interned (T&& other) {
			static_assert (!definitely_disjunct_t<typename interned::domain_t, pure::domain_t<T>>::eval ());
			switch (Var::tag_for<object_type> (other)) {
				case Var::Tag::Nil : {
					if constexpr (Nil::is_never_nil)
						throw operation_not_supported ();
					else
						this->init_nil ();
				}
					return;
				case Var::Tag::Interned : this->init_ptr (Var::Tag::Interned, Var::obj (other));
					return;
				case Var::Tag::String :
					if constexpr (Var::has_interned<T>) {
						this->init_ptr (Var::Tag::Interned, Var::obj (other));
						return;
					}
				default : throw operation_not_supported ();
					return;
			}
		}
		interned (const interned& other) { this->value = other.value; }

		~interned () { this->init_nil (); }

		Var::Tag tag () const noexcept {
			if constexpr (Nil::is_never_nil) return Var::Tag::Interned;
			else return this->value ? Var::Tag::Interned : Var::Tag::Nil;
		}
	};
}
