/*
 *  $Id: $
 *
 *  Copyright 2026 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  \brief CPU/cache utilities; C++20
 *
 */

#ifndef INCLUDED_cpu_hpp_761c5125_5c94_4fa7_9819_cdf017d08a41
#define INCLUDED_cpu_hpp_761c5125_5c94_4fa7_9819_cdf017d08a41

#pragma once


#include <source_location>
#include <tuple>
#include <type_traits>
#include <utility>

#include <tuple>
#include <utility>
#include <type_traits>


namespace lpt {

namespace cpu {

namespace impl {

template <typename T, std::size_t N> struct to_tuple {};

template <typename T>
struct to_tuple<T, 3>
{
    constexpr auto operator()(T&& obj)
    {
        auto&& [a, b, c] = std::forward<T>(obj);
        return std::forward_as_tuple(
            std::forward<decltype(a)>(a), 
            std::forward<decltype(b)>(b), 
            std::forward<decltype(c)>(c) );
        //auto&& [...mems] = std::forward<T>(obj); //C++26
        //return std::forward_as_tuple(std::forward<decltype(mems)>(mems)...);
    }
};

template <typename T>
struct to_tuple<T, 2>
{
    constexpr auto operator()(T&& obj)
    {
        auto&& [a, b] = std::forward<T>(obj);
        return std::forward_as_tuple(
            std::forward<decltype(a)>(a), 
            std::forward<decltype(b)>(b) );
    }
};


// Constexpr function to verify if tuple elements are sorted by size (largest to smallest)
template <typename Tuple, std::size_t... Is>
constexpr bool is_size_sorted_impl(std::index_sequence<Is...>) 
{
    // Check if sizeof(Element N) >= sizeof(Element N+1) for all adjacent pairs
    return ((sizeof(std::tuple_element_t<Is, Tuple>) >= sizeof(std::tuple_element_t<Is + 1, Tuple>)) && ...);
}

template <typename Tuple>
constexpr bool is_size_sorted() 
{
    constexpr std::size_t Size = std::tuple_size_v<Tuple>;
    if constexpr (Size <= 1) {
        return true;
    } else {
        return is_size_sorted_impl<Tuple>(std::make_index_sequence<Size - 1>{});
    }
}


// Universal converter type
struct any_type {
    template <typename T> operator T();
};

// Count fields by checking how many 'any_type' constructors fit
template <typename T, typename... Args>
constexpr std::size_t count_fields() 
{
    if constexpr (requires { T{ {Args{}}..., {any_type{}} }; }) {
        return count_fields<T, Args..., any_type>();
    } else {
        return sizeof...(Args);
    }
}

// N-deduction wrapper
template <typename T>
constexpr auto make_tuple_from_struct(T&& obj) 
{
    using DecayedT = std::decay_t<T>;
    constexpr std::size_t N = count_fields<DecayedT>();
    return to_tuple<DecayedT, N>{}(std::forward<T>(obj));
}


template <typename Type> struct UnoptimizedTypePrinter; // Force type printing

} // namespace impl


/*
 * static_assert(lpt::cpu::is_cache_optimal<OptimizedStruct>());
 */ 
template <typename Type>
constexpr bool is_cache_optimal() 
{
    using TupleType = decltype(impl::make_tuple_from_struct(std::declval<Type>()));
    constexpr bool isOptimal = impl::is_size_sorted<TupleType>();
    if constexpr ( ! isOptimal) {
        impl::UnoptimizedTypePrinter<Type> err;
        static_assert(isOptimal, "Sort members from largest to smallest");
    }
    return isOptimal;
}


} // namespace cpu

} // namespace lpt


#endif //#define INCLUDED_cpu_hpp_761c5125_5c94_4fa7_9819_cdf017d08a41
