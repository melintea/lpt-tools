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


#include <iostream>


/**
 * iostream tools
 */

namespace lpt {

/// auto-indent ostream
// TODO: https://stackoverflow.com/questions/1391746/how-to-easily-indent-output-to-ofstream
/*

struct autoindent : private std::streambuf
                  , public std::ostream
{
    autoindent() : std::ostream(this) {}

private:

    int overflow(int c) override
    {
        foo(c);
        return 0;
    }


    void foo(char c)
    {
        std::cout.put(c);

    }
};

int main()
{
    autoindent b;
    b<<"Look a number: "<<std::hex<<29<<std::endl;

    return 0;
}

*/

} //namespace lpt


#endif //#define INCLUDED_iostream_hpp_16afdfdb_5c40_473f_9d2c_5fd3cf8fb6f6
