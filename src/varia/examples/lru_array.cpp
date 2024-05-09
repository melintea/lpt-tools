/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need a C++0x compiler.
 *
 */

#include <lpt/lru_array.hpp>

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;


struct Item
{
    std::string _tag;
    int          _val;
}; 

int main()
{
    lpt::lru_array<Item, 5> lru;
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 0);

    lru.push({"A", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 1);
    std::this_thread::sleep_for(10ms);

    lru.push({"B", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 2);
    std::this_thread::sleep_for(10ms);

    lru.push({"C", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 3);
    std::this_thread::sleep_for(10ms);

    lru.push({"D", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 4);
    std::this_thread::sleep_for(10ms);

    lru.push({"E", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    std::this_thread::sleep_for(10ms);

    lru.push({"F", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    std::this_thread::sleep_for(10ms);

    return EXIT_SUCCESS;
}
