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

#include <array>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <cxxabi.h>
#include <cstdlib>

namespace lpt {

namespace impl {

template <std::uint16_t N>
class [[nodiscard]] constexpr_string 
{

public:

    using value_type      = const char;
    using size_type       = std::uint16_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = const char*;
    using const_pointer   = const char*;
    using reference       = const char&;
    using const_reference = const char&;

    using iterator       = const char*;
    using const_iterator = const char*;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr explicit constexpr_string(std::string_view str) noexcept 
        : constexpr_string{str, std::make_integer_sequence<std::uint16_t, N>{}} 
    {
        assert(str.size() > 0 && str.size() == N);
    }

    constexpr constexpr_string() = delete;

    constexpr constexpr_string(const constexpr_string&) = default;

    constexpr constexpr_string(constexpr_string&&) = default;

    ~constexpr_string() = default;

    constexpr_string& operator=(const constexpr_string&) = default;

    constexpr_string& operator=(constexpr_string&&) = default;

    [[nodiscard]] constexpr const_pointer data() const noexcept { return chars_; }

    [[nodiscard]] constexpr size_type size() const noexcept { return N; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + size(); }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return end(); }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return begin(); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { return assert(i < size()), chars_[i]; }

    [[nodiscard]] constexpr const_reference front() const noexcept { return chars_[0]; }

    [[nodiscard]] constexpr const_reference back() const noexcept { return chars_[N]; }

    [[nodiscard]] constexpr size_type length() const noexcept { return size(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return false; }

    [[nodiscard]] constexpr int compare(std::string_view str) const noexcept { return std::string_view{data(), size()}.compare(str); }

    [[nodiscard]] constexpr const char* c_str() const noexcept { return data(); }

    [[nodiscard]] std::string str() const { return {begin(), end()}; }

    [[nodiscard]] constexpr operator std::string_view() const noexcept { return {data(), size()}; }

    [[nodiscard]] constexpr explicit operator const_pointer() const noexcept { return data(); }

    [[nodiscard]] explicit operator std::string() const { return {begin(), end()}; }

private:

    template <std::uint16_t... I>
    constexpr constexpr_string(std::string_view str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_{str[I]..., '\0'} {}

    char chars_[static_cast<std::size_t>(N) + 1];
};

template <>
class [[nodiscard]] constexpr_string<0> 
{

public:

    using value_type      = const char;
    using size_type       = std::uint16_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = const char*;
    using const_pointer   = const char*;
    using reference       = const char&;
    using const_reference = const char&;

    using iterator       = const char*;
    using const_iterator = const char*;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr explicit constexpr_string(std::string_view) noexcept {}

    constexpr constexpr_string() = default;

    constexpr constexpr_string(const constexpr_string&) = default;

    constexpr constexpr_string(constexpr_string&&) = default;

    ~constexpr_string() = default;

    constexpr_string& operator=(const constexpr_string&) = default;

    constexpr_string& operator=(constexpr_string&&) = default;

    [[nodiscard]] constexpr const_pointer data() const noexcept { return nullptr; }

    [[nodiscard]] constexpr size_type size() const noexcept { return 0; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return nullptr; }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return nullptr; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return nullptr; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return nullptr; }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return {}; }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return {}; }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return {}; }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return {}; }

    [[nodiscard]] constexpr size_type length() const noexcept { return 0; }

    [[nodiscard]] constexpr bool empty() const noexcept { return true; }

    [[nodiscard]] constexpr int compare(std::string_view str) const noexcept { return std::string_view{}.compare(str); }

    [[nodiscard]] constexpr const char* c_str() const noexcept { return nullptr; }

    [[nodiscard]] std::string str() const { return {}; }

    [[nodiscard]] constexpr operator std::string_view() const noexcept { return {}; }

    [[nodiscard]] constexpr explicit operator const_pointer() const noexcept { return nullptr; }

    [[nodiscard]] explicit operator std::string() const { return {}; }
};

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator==(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) == 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator==(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) == 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator!=(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) != 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator!=(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) != 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator>(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) > 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator>(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) > 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator>=(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) >= 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator>=(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) >= 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator<(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) < 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator<(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) < 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator<=(const constexpr_string<N>& lhs, std::string_view rhs) noexcept 
{
    return lhs.compare(rhs) <= 0;
}

template <std::uint16_t N>
[[nodiscard]] constexpr bool operator<=(std::string_view lhs, const constexpr_string<N>& rhs) noexcept 
{
    return lhs.compare(rhs) <= 0;
}

template <typename Char, typename Traits, std::uint16_t N>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, const constexpr_string<N>& srt) 
{
    for (const auto c : srt) 
    {
        os.put(c);
    }
    return os;
}

constexpr std::string_view pretty_name(std::string_view name, bool remove_suffix = true) noexcept 
{
    return name;
}

template <typename E, E V>
constexpr auto idx() noexcept 
{
    static_assert(std::is_enum_v<E>, "not an enum");

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
requires (std::is_enum_v<std::decay_t<decltype(V)>)
[[nodiscard]] constexpr auto to_string() noexcept 
{
  using D = std::decay_t<decltype(V)>;
  constexpr std::string_view name = impl::enum_name_v<D, V>;
  static_assert(!name.empty(), "enum value has no name");
  return name;
}

/*
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
