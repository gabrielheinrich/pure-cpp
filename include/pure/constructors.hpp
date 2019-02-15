#pragma once

#include <pure/support/tuple.hpp>
#include <pure/object/persistent_map.hpp>
#include <pure/object/persistent_vector.hpp>

/**
 * @brief The root namespace of Pure C++
 */
namespace pure {
	/**
	 Constructs a possibly heterogeneous vector containing the listed arguments. For small number of arguments this
	 will return a tuple with a per index type. Otherwise a Persistent::Vector<var> is returned.
	 */
	template<typename... Args>
	auto VEC (Args&& ... args) {
		if constexpr (sizeof... (Args) <= 6) {
			return make_tuple (std::forward<Args> (args)...);
		}
		else {
			return Persistent::make_vector (std::forward<Args> (args)...);
		}
	}

	/**
	 Constructs a Mapping of keys to values. If all keys are constexpr strings, this will return a struct-like record.
	 Otherwise a Persistent::Map<var, var> is returned.
	 @param args key, value, ....
	 */
	template<typename... Args>
	auto MAP (Args&& ... args) {
		if constexpr (sizeof... (Args) != 0 && detail::is_record_init<Args...>) {
			return make_record (std::forward<Args> (args)...);
		}
		else {
			return Persistent::make_map (std::forward<Args> (args)...);
		}
	}
}
