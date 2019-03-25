#pragma once

#if defined (_MSC_VER)
#define __CPP_STD_ID _MSVC_LANG
#else
#define __CPP_STD_ID __cplusplus
#endif

#if __CPP_STD_ID <= 201402L
#error "Pure C++ requires C++ 17 or higher. Please add the corresponding flags to your compiler invocation"
#endif

#include <pure/support/endian.hpp>
#include <pure/support/type_utilities.hpp>
#include <pure/support/utf8.hpp>
#include <pure/support/record.hpp>

#include <pure/type_class.hpp>
#include <pure/symbolic_sets.hpp>
#include <pure/exceptions.hpp>
#include <pure/traits.hpp>

#include <pure/types/any.hpp>
#include <pure/types/var.hpp>
#include <pure/types/restrict.hpp>
#include <pure/types/some.hpp>
#include <pure/types/maybe.hpp>
#include <pure/types/unique.hpp>
#include <pure/types/shared.hpp>
#include <pure/types/weak.hpp>
#include <pure/types/immediate.hpp>
#include <pure/types/string.hpp>

#include <pure/object/interface.hpp>
#include <pure/object/boxed.hpp>
#include <pure/object/basic_string.hpp>
#include <pure/object/persistent_vector.hpp>
#include <pure/object/persistent_map.hpp>

#include <pure/support/identifier.hpp>

#include <pure/impl/Trait_Value.hpp>
#include <pure/impl/Trait_Static_Disjunct.hpp>
#include <pure/impl/Trait_Type_Variants.hpp>
#include <pure/impl/Trait_Type_Union.hpp>
#include <pure/impl/Trait_To_Var.hpp>
#include <pure/impl/Trait_From_Var.hpp>
#include <pure/impl/Trait_CString.hpp>

#include <pure/impl/Trait_Compare.hpp>
#include <pure/impl/Trait_Hash.hpp>
#include <pure/impl/Trait_Arithmetic.hpp>

#include <pure/impl/Trait_Functional.hpp>
#include <pure/impl/Trait_Enumerable.hpp>

#include <pure/impl/Trait_Error.hpp>

#include <pure/impl/Trait_Print.hpp>
#include <pure/impl/Trait_Stream.hpp>
#include <pure/impl/Trait_Object.hpp>

#include <pure/support/string_builder.hpp>

#include <pure/constructors.hpp>
#include <pure/functions.hpp>
#include <pure/macros.hpp>
