#pragma once

#include <pure/types/some.hpp>

namespace pure {
	/**
	 	Same as some<O, maybe_nil>
	 */
	template<typename O = Interface::Value>
	using maybe = some<O, maybe_nil>;
}
