#pragma once

#include <pure/traits.hpp>

namespace pure {
	namespace Interface { struct Value; }

	struct init_tag {};
	constexpr init_tag init {};

	template<typename T, typename... Args>
	constexpr intptr_t create_capacity_needed (const Args& ... args) {
		if constexpr (T::has_dynamic_size || std::is_same_v<T, Interface::Value>)
			return T::capacity_needed (args...);
		else
			return 0;
	};

	template<typename T>
	constexpr intptr_t create_capacity_to_num_bytes (intptr_t capacity) {
		if constexpr (T::has_dynamic_size || std::is_same_v<T, Interface::Value>)
			return T::capacity_to_num_bytes (capacity);
		else
			return sizeof (T);
	}

	template<typename T, typename... Args>
	constexpr intptr_t create_num_bytes_needed (const Args& ... args) {
		return create_capacity_to_num_bytes<T> (create_capacity_to_num_bytes<T> (args...));
	}

	template<typename T, typename... Args>
	T* create_placement (void* memory, intptr_t capacity, Args&& ... args) {
		if constexpr (T::has_dynamic_size || std::is_same_v<T, Interface::Value>)
			return new (memory) T {capacity, std::forward<Args> (args)...};
		else
			return new (memory) T {std::forward<Args> (args)...};
	};

	template<typename T, typename... Args>
	T* create (Args&& ... args) {
		if constexpr (std::is_same_v<Interface::Value, T> && sizeof... (Args) == 1) {
			return Var::clone (std::forward<Args> (args)...);
		}
		else if constexpr (T::has_dynamic_size) {
			auto capacity = T::capacity_needed (args...);
			return new (T::capacity_to_num_bytes (capacity)) T {capacity, std::forward<Args> (args)...};
		}
		else {
			return new T {std::forward<Args> (args)...};
		}
	}

	template<typename T>
	T* create (std::nullptr_t) {
		assert (0);
		return nullptr;
	}
}