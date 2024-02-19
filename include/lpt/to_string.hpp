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

#include <lpt/constexpr_string.hpp>

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <cxxabi.h>
#include <cstdlib>

namespace lpt {

namespace impl {

// constexpr auto lpt::impl::name_and_val() [with E = main()::Color; E V = main::Color::eRED]
constexpr std::string_view enum_val_name(std::string_view name) noexcept 
{
    int i(name.length() - 1);
    for (i; i >=0; --i) 
    {
        if (name[i] == ' ')
        {
            break;
        }
    }

    return std::string_view(&name[i], name.length() -  1 - i); // main::Color::eRED
}

template <typename E, E V>
constexpr auto name_and_val() noexcept 
{
    static_assert(std::is_enum_v<E>, "not an enum");
   return enum_val_name(std::source_location::current().function_name());
}

template <typename E, E V>
constexpr auto enum_name() noexcept 
{
    constexpr auto name = name_and_val<E, V>();
    return std::string_view{name}; //constexpr_string<name.size()>{name};
}

template <typename E, E V>
inline constexpr auto enum_name_v = enum_name<E, V>();

}  // namespace impl


/*
 * Stringizing an enum:
 */

template <auto V>
requires (std::is_enum_v<decltype(V)>)
[[nodiscard]] constexpr auto to_string() noexcept 
{
    using D = std::decay_t<decltype(V)>;
    //using U = std::underlying_type_t<D>;
    constexpr std::string_view name = impl::enum_name_v<D, V>;
    static_assert( ! name.empty(), "enum value has no name");
    return name;
}

/*
WIP

template <typename E>
requires (std::is_enum_v<E>)
constexpr auto to_string(E v)
{
    using D = std::decay_t<E>;
    using U = std::underlying_type_t<D>;
    const auto val - static_cast<U>(v);
    constexpr std::string_view name = impl::enum_name_v<D, V>;

    return impl::to_string<D>(v);
}
*/

} //namespace lpt


#endif //#define INCLUDED_to_string_hpp_c9ff19fd_9f4e_403a_a3c3_060c3309ceca
