/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need a C++0x compiler.
 *
 */

#ifndef INCLUDED_lru_array_hpp_025884f0_0423_46a1_bf95_5481fe40426b
#define INCLUDED_lru_array_hpp_025884f0_0423_46a1_bf95_5481fe40426b

#pragma once

#include <array>
#include <cassert>
#include <chrono>
#include <deque>>
#include <functional>
#include <iostream>
#include <queue>
#include <unordered_map>

namespace lpt {

/*
 * Store a specified max number of items, replace the oldest 
 * when full.
 */
template <typename T, std::size_t N>
class lru_array
{
private: 

    struct Item
    {
        T       _data;

        using clock_t = std::chrono::high_resolution_clock;
        using stamp_t = clock_t::time_point;
        stamp_t _stamp;

        Item()   = default;
        Item(const T& data) 
            : _data(data)
            , _stamp(clock_t::now())
        {}
        ~Item()  = default;

        Item( const Item& other )             = default;
        Item& operator=( const Item& other )  = default;

        Item( Item&& other )                  = default;
        Item& operator=( Item&& other )       = default;
    };

public:

    lru_array()   = default;
    ~lru_array()  = default;

    lru_array( const lru_array& other )            = default;
    lru_array& operator=( const lru_array& other ) = default;

    lru_array( lru_array&& other )                 = default;
    lru_array& operator=( lru_array&& other )      = default;

    // Note: std::array::size() == N
    constexpr size_t size() const noexcept
    {
        return _numUsed;
    }

    constexpr size_t max_size() const noexcept
    {
        return _data.max_size();
    }

    void push(const T& data)
    {
        if (_numUsed < max_size()) {
            _data[_numUsed] = data;
            _idxLru.push(IdxItem{_data[_numUsed], _numUsed});
            ++_numUsed;
        } else {
            auto idx(_idxLru.top()._idx);
            assert(idx < max_size());

            _data[idx] = data;
            _idxLru.pop();
            _idxLru.push(IdxItem{_data[idx], idx});
        }

        //std::cout << "*** " << _numUsed << " *** " << _idxLru.size() << "\n";
        assert(_numUsed == _idxLru.size());
    }

private: 

    std::array<Item, N> _data;
    size_t              _numUsed{0};

    struct IdxItem
    {
        std::reference_wrapper<Item> _data;
        size_t                       _idx; // in _data
    }; 

    using lruidx_t = std::priority_queue<
                        IdxItem,
                        std::deque<IdxItem>,
                        decltype([](const IdxItem& lhs, const IdxItem& rhs){
                            return lhs._data.get()._stamp < rhs._data.get()._stamp;
                        })
                     >;
    lruidx_t           _idxLru;

}; // lru_array

} //namespace lpt


#endif //#define INCLUDED_lru_array_hpp_025884f0_0423_46a1_bf95_5481fe40426b
