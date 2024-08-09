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


#include <string>
#include <iostream>


/**
 * iostream tools
 */

namespace lpt {

/// Poor performance auto-indent ostream
///  @see also https://stackoverflow.com/questions/1391746/how-to-easily-indent-output-to-ofstream

struct autoindent : private std::streambuf
                  , public std::ostream
{
    autoindent(std::ostream& os) 
        : std::ostream(this) 
	, _os(os)
    {}

    autoindent()   = delete;
    ~autoindent()  = default;

    autoindent( const autoindent& other )            = default;
    autoindent& operator=( const autoindent& other ) = default;

    autoindent( autoindent&& other )                 = default;
    autoindent& operator=( autoindent&& other )      = default;

private:

    int overflow(int c) override
    {
        _os.put(c);
	if (c == '\n') {
	    _os << sc_indent;
	}
	
        return 0;
    }

    std::ostream& _os;
    int           _level{0};
    
    static constexpr const char* sc_indent = "    ";
};


} //namespace lpt


#endif //#define INCLUDED_iostream_hpp_16afdfdb_5c40_473f_9d2c_5fd3cf8fb6f6
