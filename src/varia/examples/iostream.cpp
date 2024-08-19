/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need a C++0x compiler.
 *
 */

#include <lpt/iostream.hpp>

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

struct l3
{
    l3(const std::vector<std::string>& data) : _data(data)    {}
    l3(std::initializer_list<std::string> data) : _data(data) {}
    
    l3()   = default;
    ~l3()  = default;

    l3( const l3& other )            = default;
    l3& operator=( const l3& other ) = default;

    l3( l3&& other )                 = default;
    l3& operator=( l3&& other )      = default;
    
    std::vector<std::string> _data;
    
    friend std::ostream& operator<<(std::ostream& os, const l3& x)
    {
        lpt::autoindent_guard indent(os);
        os << "(l3\n";
        std::ranges::copy(x._data, std::ostream_iterator<std::string>(os, "\n"));
        os << ")l3";
        return os;
    }
};

struct l2
{
    l2(const std::vector<l3>& data) : _data(data)    {}
    l2(std::initializer_list<l3> data) : _data(data) {}
    
    l2()   = default;
    ~l2()  = default;

    l2( const l2& other )            = default;
    l2& operator=( const l2& other ) = default;

    l2( l2&& other )                 = default;
    l2& operator=( l2&& other )      = default;
    
    std::vector<l3> _data;
    
    friend std::ostream& operator<<(std::ostream& os, const l2& x)
    {
        lpt::autoindent_guard indent(os);
        os << "(l2\n";
        std::ranges::copy(x._data, std::ostream_iterator<l3>(os, "\n"));
        os << ")l2";
        return os;
    }
};

struct l1
{
    l1(const std::vector<l2>& data) : _data(data)    {}
    l1(std::initializer_list<l2> data) : _data(data) {}
    
    l1()   = default;
    ~l1()  = default;

    l1( const l1& other )            = default;
    l1& operator=( const l1& other ) = default;

    l1( l1&& other )                 = default;
    l1& operator=( l1&& other )      = default;
    
    std::vector<l2> _data;
    
    friend std::ostream& operator<<(std::ostream& os, const l1& x)
    {
        lpt::autoindent_guard indent(os);
        os << "(l1\n";
        std::ranges::copy(x._data, std::ostream_iterator<l2>(os, "\n"));
        os << ")l1";
        return os;
    }
};

int main()
{
    l1 data{
        {
            {"l1.1-l2.1-l3.1", "l1.1-l2.1-l3.2", "l1.1-l2.1-l3.3"}
        },
        {
            {"l1.1-l2.2-l3.1", "l1.1-l2.2-l3.2", "l1.1-l2.2-l3.3"}
        }, 
        {
            {"l1.1-l2.3-l3.1", "l1.1-l2.3-l3.2", "l1.1-l2.3-l3.3"}
        }
    };
    
    {
        std::cout << "[== std::cout ===" << std::endl;
        std::cout << "\nLook a number: " <<std::hex<<29 << std::endl;
        std::cout << "\nl1: " << data << std::endl;
        std::cout << "]== std::cout ===" << std::endl;
    }
    {
        std::cout << "[== autoindent ===" << std::endl;
        lpt::autoindent_ostream ios(std::cout);
        ios << "\nLook a number: " <<std::hex<<29 <<std::endl;
        ios << "\nl1:\n" << data << std::endl;
        std::cout << "]== autoindent ===" << std::endl;
    }

    return 0;
}

