#pragma once

#include <pure/types/some.hpp>
#include <pure/support/create.hpp>

namespace pure {
	/**
	 Value holder, which allocates a value in place, right next to the pointer itself.
	 @tparam O Type of the held value. Has to have Interface::Value as a base class.
	 @tparam Capacity Optional capacity needed for variable sized Value types like Basic::String
	 */
	template<typename O, intptr_t Capacity = 0>
	struct immediate : some<O, never_nil> {
		using object_type = O;
		using domain_t = typename O::domain_t;

		static constexpr bool definitely_mutable_pointer = true;

		static constexpr intptr_t Byte_Capacity = create_capacity_to_num_bytes<O> (Capacity);

		alignas (O) char memory[Byte_Capacity];

		template<typename... Args>
		immediate (Args&& ... args) {
			if constexpr (object_type::has_dynamic_size || std::is_same_v<Interface::Value, object_type>) {
				auto b = object_type::capacity_to_num_bytes (object_type::capacity_needed (args...));
				if (b <= Byte_Capacity) {
					this->init_ptr (Var::Tag::Moveable, new (memory) object_type {b, std::forward<Args> (args)...});
				}
				else {
					throw operation_not_supported ();
				}
			}
			else {
				this->init_ptr (Var::Tag::Moveable, new (memory) object_type {std::forward<Args> (args)...});
			}
		}

		immediate (const immediate& other) : immediate {static_cast<const immediate&&> (other)} {}


		~immediate () {
			reinterpret_cast<object_type*>(memory)->~object_type ();
			this->init_nil ();
		}

		Var::Tag tag () const noexcept { return Var::Tag::Moveable; }
	};

}
