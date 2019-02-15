#pragma once

#include <pure/types/some.hpp>

namespace pure {
	/**
	 Stable pointer to a value. The value is allocated on the heap via new and is owned by this pointer. Because the
	 value is uniquely owned it can be mutated.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Nil Either never_nil or maybe_nil to specify whether nullptr should be included in the domain of this type.
	 */
	template<typename O = Interface::Value, typename Nil = never_nil>
	struct unique : some<O, Nil> {
		using object_type = O;
		static constexpr bool definitely_mutable_pointer = true;

		unique (O* ptr) { this->init_ptr (Var::Tag::Unique, ptr); }

		template<typename T>
		unique (T&& other) {
			static_assert (!definitely_disjunct_t<typename unique::domain_t, pure::domain_t<T>>::eval ());
			switch (Var::tag_for<object_type> (other)) {
				case Var::Tag::Nil : {
					if constexpr (Nil::is_never_nil)
						throw operation_not_supported ();
					else
						this->init_nil ();
				}
					return;
				case Var::Tag::Unique :
					if constexpr (detail::is_moveable<T&&>) {
						this->init_ptr (Var::Tag::Unique, Var::release (other));
						return;
					}
				case Var::Tag::Shared :
					if constexpr (detail::is_moveable<T&&>)
						if (detail::ref_count_is_unique (*Var::obj (other))) {
							this->init_ptr (Var::Tag::Unique, Var::release (other));
							return;
						}
				default : this->init_ptr (Var::Tag::Unique, create<object_type> (std::forward<T> (other)));
					return;
			}
		}
		unique (const unique& other) : unique (static_cast<const unique&&> (other)) {}

		~unique () {
			switch (this->read_tag ()) {
				case Var::Tag::Unique : delete this->operator-> ();
					break;
				default : break;
			}
			this->init_nil ();
		}

		template<typename T>
		const unique& operator= (T&& other) {
			if ((void*) &other == (void*) this) return *this;
			else {
				this->~unique ();
				this->init_nil ();
				new (this) unique {std::forward<T> (other)};
				return *this;
			}
		}

		const unique& operator= (const unique& other) { return (*this = static_cast<const unique&&> (other)); }

		Var::Tag tag () const noexcept {
			if constexpr (Nil::is_never_nil) return Var::Tag::Unique;
			else return this->value ? Var::Tag::Unique : Var::Tag::Nil;
		}
	};

	template<typename T, typename... Args>
	unique<T> make_unique (Args&& ... args) { return {create<T> (std::forward<Args> (args)...)}; };
}
