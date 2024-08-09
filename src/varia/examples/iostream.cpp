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


int main()
{
    lpt::autoindent ios(std::cout);
    ios << "\nLook a number: "<<std::hex<<29<<std::endl;

    return 0;
}

