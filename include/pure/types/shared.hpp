#pragma once

#include <pure/types/some.hpp>

namespace pure {
	/**
	 Stable pointer to a value. The value is allocated on the heap via new and its lifetime is handled via an atomic
	 refcount. The underlying value can not be mutated, because it's not guranteed to not be accessed by other
	 threads concurrently.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Nil Either never_nil or maybe_nil to specify whether nullptr should be included in the domain of this type.
	 */
	template<typename O = Interface::Value, typename Nil = never_nil>
	struct shared : some<O, Nil> {
		using object_type = O;
		static constexpr bool definitely_const_pointer = true;

		shared (O* ptr) { this->init_ptr (Var::Tag::Shared, ptr); }

		template<typename T>
		shared (T&& other) {
			static_assert (!definitely_disjunct_t<typename shared::domain_t, pure::domain_t<T>>::eval ());
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
						this->init_ptr (Var::Tag::Shared, Var::release (other));
						return;
					}
				case Var::Tag::Shared :
					if constexpr (detail::is_moveable<T&&>) {
						this->init_ptr (Var::Tag::Shared, Var::release (other));
						return;
					}
					else {
						this->init_ptr (Var::Tag::Shared, detail::inc_ref_count_and_get (*Var::obj (other)));
						return;
					}
				default : this->init_ptr (Var::Tag::Shared, create<object_type> (std::forward<T> (other)));
					return;
			}
		}
		shared (const shared& other) : shared (static_cast<const shared&&> (other)) {}

		~shared () {
			switch (this->read_tag ()) {
				case Var::Tag::Shared : detail::dec_ref_count_and_delete (this->operator* ());
					break;
				default : break;
			}
			this->init_nil ();
		}

		template<typename T>
		const shared& operator= (T&& other) {
			if ((void*) &other == (void*) this) return *this;
			else {
				this->~shared ();
				this->init_nil ();
				new (this) shared {std::forward<T> (other)};
				return *this;
			}
		}

		const shared& operator= (const shared& other) { return (*this = static_cast<const shared&&> (other)); }

		Var::Tag tag () const noexcept {
			if constexpr (Nil::is_never_nil) return Var::Tag::Shared;
			else return this->read_tag();
		}
	};

	template<typename T, typename... Args>
	shared<T> make_shared (Args&& ... args) { return {create<T> (std::forward<Args> (args)...)}; };
}
