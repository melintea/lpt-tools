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

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <deque>>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <set>
#include <unordered_map>

namespace lpt {

/*
 * As a custom priority queue
 */
template <typename T,
          typename COMPARE_T = std::less<T>
>
class ordered_set
{
public:

    const std::set<T>& data() const {return _data;}

    void push(const T& data)
    {
        if (auto it = _data.find(data); it != _data.end()) {
            _data.erase(it);
        }
        _data.emplace(data);
    }

    void pop()
    {
        _data.erase(_data.begin());
    }

    const T& top() const
    {
        return *_data.begin();
    }

    auto size() const { return _data.size(); }

    auto begin() { return _data.begin(); }
    auto end()   { return _data.end(); }
    auto cbegin() const { return _data.cbegin(); }
    auto cend() const   { return _data.cend(); }

private:

    std::set<T, COMPARE_T> _data;

}; // ordered_set

/*
 * Store a specified max number of items, replace the oldest 
 * when full.
 */
template <typename    KEY,
          typename    T, 
          std::size_t N
>
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

        friend std::ostream& operator<<(std::ostream& os, const Item& i)
        {
            os << '(' << i._data /*<< ',' << i._stamp*/ << ')';
            return os;
        }
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

    void push(const KEY& key, const T& data)
    {
        std::cout << data << ": ";

        auto itKey(_idxData.find(key));
        if (itKey != _idxData.end()) {
            auto& item = itKey->second;

            _data[item._idx] = data;
            //_idxLru.pop();  // not with ordered_set
            _idxLru.push(IdxItem{_data[item._idx], item._idx, key});

            item = IdxItem{_data[item._idx], item._idx, key};

            invariant();
            return;
        }

        if (_numUsed >= max_size()) {
            const auto& oldKey(_idxLru.top()._key);
            auto idx(_idxLru.top()._idx);
            assert(idx < max_size());
            std::cout << "out->" << oldKey;

            _data[idx] = data;
            _idxLru.pop();
            _idxLru.push(IdxItem{_data[idx], idx, key});

            _idxData.erase(oldKey);
            auto [it, inserted] = _idxData.insert(std::make_pair(key, IdxItem{_data[idx], idx, key}));
            assert(inserted);

            invariant();
            return;
        }

        _data[_numUsed] = data;
        _idxLru.push(IdxItem{_data[_numUsed], _numUsed, key});

        auto [it, inserted] = _idxData.insert(std::make_pair(key, IdxItem{_data[_numUsed], _numUsed, key}));
        assert(inserted);

        ++_numUsed;

        invariant();
    }

    friend std::ostream& operator<<(std::ostream& os, const lru_array& a)
    {
        os << "D [";
        //std::copy(a._data.cbegin(), a._data.cend(), std::ostream_iterator<T>(os, ","));
        std::for_each(a._data.cbegin(), a._data.cend(), [&os](const auto& item){ os << item;});
        os << "]\nID[";
        std::for_each(a._idxData.cbegin(), a._idxData.cend(), [&os](const auto& item){ os << item.second;});
        os << "]\nIT[";
        std::for_each(a._idxLru.cbegin(), a._idxLru.cend(), [&os](const auto& item){ os << item;});
        os << "]\n";
        return os;
    }

private:

    void invariant() const
    {
        std::cout << "*** " << _numUsed 
                  << " *** " << _idxLru.size() << '[' << _idxLru.top() << ']'
                  << " *** " << _idxData.size() << "\n";
        std::cout << *this;
        //assert(_numUsed == _idxLru.size());
        assert(_numUsed == _idxData.size());
    }

private: 

    std::array<Item, N> _data;
    size_t              _numUsed{0};

    struct IdxItem
    {
        std::reference_wrapper<Item> _data; // TODO: redundant since we have the idx?
        size_t                       _idx;  // in _data
        KEY                          _key;

        friend std::ostream& operator<<(std::ostream& os, const IdxItem& i)
        {
            os << '(' << i._key << ',' << i._idx << ')';
            return os;
        }
    }; 

    /*
    using lruidx_t = std::priority_queue<
                        IdxItem,
                        std::deque<IdxItem>,
                        decltype([](const IdxItem& lhs, const IdxItem& rhs){
                            return lhs._data.get()._stamp >= rhs._data.get()._stamp;
                        })
                     >;
    */
    using lruidx_t = ordered_set<
                        IdxItem,
                        decltype([](const IdxItem& lhs, const IdxItem& rhs){
                            return lhs._data.get()._stamp < rhs._data.get()._stamp;
                        })
                     >;
    lruidx_t           _idxLru;

    using dataidx_t = std::unordered_map<
                          KEY,
                          IdxItem,
                          std::hash<KEY>,
                          std::equal_to<KEY>
                      >;
    dataidx_t          _idxData;

}; // lru_array

} //namespace lpt


#endif //#define INCLUDED_lru_array_hpp_025884f0_0423_46a1_bf95_5481fe40426b
