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

    friend std::ostream& operator<<(std::ostream& os, const Item& i)
    {
        os << '(' << i._tag << ',' << i._val << ')';
        return os;
    }
}; 

int main()
{
    lpt::lru_array<std::string, Item, 5> lru;
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 0);

    lru.push("A", {"A", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 1);
    std::this_thread::sleep_for(10ms);

    lru.push("B", {"B", 1});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 2);
    std::this_thread::sleep_for(10ms);

    lru.push("C", {"C", 2});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 3);
    std::this_thread::sleep_for(10ms);

    lru.push("D", {"D", 3});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 4);
    std::this_thread::sleep_for(10ms);

    lru.push("E", {"E", 4});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    std::this_thread::sleep_for(10ms);

    lru.push("A", {"A", 0});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    lru.push("A", {"A", 5});
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    std::this_thread::sleep_for(10ms);

    lru.push("F", {"F", 6});  // replace B: A(5)F(6)C(2)D(3)E(4)
    static_assert(lru.max_size() == 5);
    assert(lru.size() == 5);
    std::this_thread::sleep_for(10ms);

    std::cout << '\n' << lru << '\n';

    return EXIT_SUCCESS;
}
