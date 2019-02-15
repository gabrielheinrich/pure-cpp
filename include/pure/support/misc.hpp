#pragma once

#include <cstdint>

namespace pure::detail {
	intptr_t round_up_8 (intptr_t x) {
		return (x + 7) & ~uintptr_t (7);
	}
}

#if (UINTPTR_MAX == 0xFFFFFFFF)
#define PURE_32BIT 1
#elif (UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF)
#define PURE_64BIT 1
#else
#error "Couldn't detect target architecture"
#endif

#if defined(__GNUC__)
#define PURE_COMPILER_GCC 1
#elif defined (_MSC_VER)
#define PURE_COMPILER_MSVC 1
#else
#error "Couldn't detect compiler"
#endif