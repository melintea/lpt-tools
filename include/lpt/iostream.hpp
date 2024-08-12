/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need a C++0x compiler.
 *
 */

#ifndef INCLUDED_iostream_hpp_16afdfdb_5c40_473f_9d2c_5fd3cf8fb6f6
#define INCLUDED_iostream_hpp_16afdfdb_5c40_473f_9d2c_5fd3cf8fb6f6

#pragma once


#include <cassert>
#include <string>
#include <iostream>


/**
 * iostream tools
 */

namespace lpt {

/// Poor performance indenting ostream
/// @see also https://stackoverflow.com/questions/1391746/how-to-easily-indent-output-to-ofstream
/// for better ideas

struct autoindent_ostream : private std::streambuf
                          , public std::ostream
{
    autoindent_ostream(std::ostream& os) 
        : std::ostream(this) 
	, _os(os)
    {}

    autoindent_ostream()   = delete;
    ~autoindent_ostream()  = default;

    autoindent_ostream( const autoindent_ostream& other )            = default;
    autoindent_ostream& operator=( const autoindent_ostream& other ) = default;

    autoindent_ostream( autoindent_ostream&& other )                 = default;
    autoindent_ostream& operator=( autoindent_ostream&& other )      = default;
    
    int level(int delta)
    {
        int crt(_level);
	
	_level += delta;
	if (_level < 0) {
	    _level = 0;
	}
	
	return crt;
    }
    
    void reset(int level)
    {
        assert(level >= 0);
	_level = level;
    }

private:

    int overflow(int c) override
    {
        _os.put(c);
	if (c == '\n') {
	    for (int i = 0; i <_level; ++i) {
                _os << sc_indent;
	    }
	}
	
        return 0;
    }

    std::ostream& _os;
    int           _level{0};
    
    static constexpr const char* sc_indent = "    ";

}; // autoindent_ostream


template <typename OS>
class autoindent_guard 
{
public:

    autoindent_guard(OS& ios)
        : _ios(ios)
    {}
    
    autoindent_guard(autoindent_ostream& ios)
        : _ios(ios)
    {}
    
    autoindent_guard()   = delete;
    
    ~autoindent_guard()  = default;

    autoindent_guard( const autoindent_guard& other )            = delete;
    autoindent_guard& operator=( const autoindent_guard& other ) = delete;

    autoindent_guard( autoindent_guard&& other )                 = delete;
    autoindent_guard& operator=( autoindent_guard&& other )      = delete;
    
    
private:
    
    OS&  _ios;

}; // autoindent_guard


template <>
class autoindent_guard<autoindent_ostream> 
{
public:

    autoindent_guard(autoindent_ostream& ios)
        : _ios(ios)
	, _oldlevel(_ios.level(+1))
    {}
    
    autoindent_guard()   = delete;
    
    ~autoindent_guard()
    {
        _ios.reset(_oldlevel);
    }

    autoindent_guard( const autoindent_guard& other )            = delete;
    autoindent_guard& operator=( const autoindent_guard& other ) = delete;

    autoindent_guard( autoindent_guard&& other )                 = delete;
    autoindent_guard& operator=( autoindent_guard&& other )      = delete;
    
    
private:
    
    autoindent_ostream&  _ios;
    int                  _oldlevel{0};

}; // autoindent_guard

} //namespace lpt


#endif //#define INCLUDED_iostream_hpp_16afdfdb_5c40_473f_9d2c_5fd3cf8fb6f6
