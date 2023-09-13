/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 */

#include <lpt/chrono.hpp>

#include <iostream>
#include <thread>


//-----------------------------------------------------------------------------
int main()
{
    {
        lpt::chrono::measurement tm("tm",
                                    [](auto&& tag, auto&& dur) -> decltype(auto) {
                                        std::cout << tag << ": " << dur.count() << std::endl;
                                    });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return EXIT_SUCCESS; 
}

