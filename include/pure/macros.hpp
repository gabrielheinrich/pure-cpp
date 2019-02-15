#pragma once

#ifdef FORWARD
#error "FORWARD is already defined"
#endif
/**
 @file
 @brief Macros defined in Pure C++
 */


#define FORWARD(x) std::forward<decltype(x)>(x)
/**
 @def FORWARD(x)
 Macro to forward universal references to other functions. Very useful in generic lambdas where the argument types
 are defined via auto&&.
 */
