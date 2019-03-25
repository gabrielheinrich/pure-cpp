#pragma once

#include <pure/types/var.hpp>
#include <pure/support/ref_count.hpp>
#include <pure/support/create.hpp>

namespace pure {
	/**
	 Marker to be used as a template argument to value holder types, to specify that nullptr is not a possible value.
	 */
	struct never_nil { static constexpr bool is_never_nil = true; };

	/**
	 Marker to be used as a template argument to value holder types, to specify that nullptr is a possible value.
	 */
	struct maybe_nil { static constexpr bool is_never_nil = false; };

	/**
	 Variant pointer to a value. The ownership of the value is handled dynamically.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Nil Either never_nil or maybe_nil to specify whether nullptr should be included in the domain of this type.
	 */
	template<typename O = Interface::Value, typename Nil = never_nil>
	struct some : var {
		using object_type = O;

		using domain_t = std::conditional_t<Nil::is_never_nil, typename object_type::domain_t,
				Union_t<typename object_type::domain_t, Nil_t>>;

		static constexpr bool maybe_nil = !Nil::is_never_nil;
		static constexpr bool definitely_pointer = true;

		constexpr some () = default;
		template<typename T>
		some (T&& other) {
			static_assert (!definitely_disjunct_t<domain_t, pure::domain_t<T>>::eval ());
			using namespace Var;
			switch (Var::tag_for<object_type> (other)) {
				case Tag::Nil : {
					if constexpr (Nil::is_never_nil)
						throw operation_not_supported ();
					else
						this->init_nil ();
				}
					return;
				case Tag::Unique : {
					if constexpr (detail::is_moveable<T&&>)
						this->init_ptr (Tag::Unique, Var::release (other));
					else
						this->init_ptr (Tag::Shared, create<object_type> (std::forward<T> (other)));
				}
					return;
				case Tag::Shared : {
					if constexpr (detail::is_moveable<T&&>)
						this->init_ptr (Tag::Shared, Var::release (other));
					else
						this->init_ptr (Tag::Shared, detail::inc_ref_count_and_get (*Var::obj (other)));
				}
					return;
				case Tag::Interned : this->init_ptr (Tag::Interned, Var::obj (other));
					return;
				case Tag::String :
					if constexpr (Var::has_interned_for<object_type, std::decay_t<T>>) {
						init_ptr (Tag::Interned, Var::obj (other));
						return;
					}
				default : init_ptr (Tag::Shared, create<object_type> (std::forward<T> (other)));
			}
		}
		some (const some& other) : some {static_cast<const some&&> (other)} {}
		~some () {
			switch (this->read_tag ()) {
				case Var::Tag::Unique : delete this->operator-> ();
					break;
				case Var::Tag::Shared : detail::dec_ref_count_and_delete (this->operator* ());
					break;
				default: break;
			}
			this->init_nil ();
		}

		template<typename T>
		const some& operator= (T&& other) {
			if ((void*) &other == (void*) this) return *this;
			else {
				this->~some ();
				this->init_nil ();
				new (this) some {std::forward<T> (other)};
				return *this;
			}
		}

		const some& operator= (const some& other) { return (*this = static_cast<const some&&> (other)); }

		Var::Tag tag () const noexcept {
			using namespace Var;
			switch (this->read_tag ()) {
				case Tag::Nil : return Nil::is_never_nil ? assert (0), Tag::Interned : Tag::Nil;
				case Tag::Unique : return Tag::Unique;
				case Tag::Shared : return Tag::Shared;
				case Tag::Interned : return Tag::Interned;
				case Tag::Moveable : return Tag::Moveable;
				default : {
					assert (0);
					return Tag::Interned;
				}
			}
		}

		object_type* operator-> () const noexcept { return static_cast<object_type*>(this->read_ptr ()); }
		const object_type& operator* () const noexcept { return *this->operator-> (); }
		object_type& operator* () noexcept { return *this->operator-> (); }

		object_type* release () noexcept { return static_cast<object_type*>(static_cast<var*>(this)->release ()); }
	};
}
