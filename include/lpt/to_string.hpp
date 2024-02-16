/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  Enum stringizing.
 *  You need a C++20 compiler.
 *
 */

#ifndef INCLUDED_to_string_hpp_c9ff19fd_9f4e_403a_a3c3_060c3309ceca
#define INCLUDED_to_string_hpp_c9ff19fd_9f4e_403a_a3c3_060c3309ceca

#pragma once

#include <string>
#include <type_traits>

namespace lpt {

namespace impl {

}  // namespace impl

/*
 * Stringizing an enum:
 */
template <typename E>
requires (std::is_enum_v<E>)
auto to_string(E v)
{
    return std::string("here");
}

} //namespace lpt


#endif //#define INCLUDED_to_string_hpp_c9ff19fd_9f4e_403a_a3c3_060c3309ceca
