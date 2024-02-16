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
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <cxxabi.h>
#include <cstdlib>

namespace lpt {

namespace impl {

constexpr std::string_view pretty_name(std::string_view name) noexcept 
{
    return name;
}

template <typename E, E V>
constexpr auto idx() noexcept 
{
    static_assert(std::is_enum_v<E>, "not an enum");

    //TODO: use std::source_location::current().function_name();

#if defined(__clang__) || defined(__GNUC__)
    constexpr auto name = pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
#elif defined(_MSC_VER)
    constexpr auto name = pretty_name({__FUNCSIG__, sizeof(__FUNCSIG__) - 17});
#else
    constexpr auto name = std::string_view{};
#endif
    return name;
}

template <typename E, E V>
constexpr auto enum_name() noexcept 
{
    constexpr auto name = idx<E, V>();
    return constexpr_string<name.size()>{name};
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
    //constexpr std::string_view name = detail::enum_name_v<D, V>;

    return impl::to_string<D>(v);
}
*/

} //namespace lpt


#endif //#define INCLUDED_to_string_hpp_c9ff19fd_9f4e_403a_a3c3_060c3309ceca
