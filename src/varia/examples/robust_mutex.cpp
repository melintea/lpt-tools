/*
 * Enum stringizing 
 */
 

#include <lpt/robust_mutex.hpp>

#include <cassert>
#include <iostream>

int main()
{
    lpt::robust_mutex mtx;
    
    {
        std::lock_guard g(mtx);
    }
    
    {
        std::lock_guard g(mtx);
    }

    return EXIT_SUCCESS;
}
