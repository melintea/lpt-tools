/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  Iterate over a tuple
 *  You need a C++20 compiler.
 *
 */

#ifndef INCLUDED_for_each_hpp_15fb8c43_aea8_44e7_8f1c_7f85c4f8a9a9
#define INCLUDED_for_each_hpp_15fb8c43_aea8_44e7_8f1c_7f85c4f8a9a9

#pragma once


#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>


namespace lpt {


namespace impl
{

template <std::size_t ... IDXs>
constexpr auto make_reversed_index_sequence(std::index_sequence<IDXs...> const &)
    -> decltype( std::index_sequence<sizeof...(IDXs)-1U-IDXs...>{} );


template <typename TUPLE, typename F, std::size_t ...IDXs>
void for_each(TUPLE&& tuple, F&& func, std::index_sequence<IDXs...>)
{
    using forceInst = int[];
    if constexpr (sizeof...(IDXs) > 0)
    {
        (void)forceInst{ ( func(std::get<IDXs>(std::forward<TUPLE>(tuple))), 
	                       void,
                           int{} )...  
        };
    }
}

} // impl

template <std::size_t N>
using make_reversed_index_sequence  = decltype(impl::make_reversed_index_sequence(std::make_index_sequence<N>{}));


/**
 *  Iterate over a tuple:
 *  \code
 *    for_each(std::make_tuple(1, 'X', 3.14), [](auto x) {
 *        std::cout << x << std::endl; // 1 X 3.14
 *    });
 *    reverse_for_each(std::make_tuple(1, 'Y', 3.14)), [](auto x) {
 *        std::cout << x << std::endl; // 3.14 Y 1
 *    });
 *  \endcode
 */
template <typename TUPLE, typename F>
void for_each(TUPLE&& tuple, F&& func)
{
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<TUPLE>>::value;
    impl::for_each(std::forward<TUPLE>(tuple),
                   std::forward<F>(func),
                   std::make_index_sequence<N>{});
}

template <typename TUPLE, typename F>
void reverse_for_each(TUPLE&& tuple, F&& func)
{
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<TUPLE>>::value;
    impl::for_each(std::forward<TUPLE>(tuple),
                   std::forward<F>(func),
                   make_reversed_index_sequence<N>{});
}

} //namespace lpt


#endif //#define INCLUDED_for_each_hpp_15fb8c43_aea8_44e7_8f1c_7f85c4f8a9a9
