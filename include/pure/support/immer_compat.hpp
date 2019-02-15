#pragma once

#include <pure/support/misc.hpp>

#if defined (PURE_COMPILER_MSVC)
#if defined (PURE_32BIT)
#define __popcnt64(x) (int16_t)(x)
#endif
#pragma warning (disable : 4996 4521)
#endif
