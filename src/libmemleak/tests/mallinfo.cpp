/*
 *  Copyright 2012 Aurelian Melinte. 
 *  Released under GPL 3.0 or later. 
 *
 *  The mallinfo() API was deprecated. 
 * 
 */
 


#include <iostream>
#include <string>
#include <cassert>

#include <lpt/callstack/mallinfo.hpp>


int main()
{
 
    lpt::process::memory first;
 
    {
        void* p = ::malloc(1025);
        lpt::process::memory second;
        std::cout << "Mem diff: " << second.total() - first.total() << std::endl;
        // Surprise: gcc 4.7.0 does not see the malloc!  Otherwise the condition should be second > first
        assert(second >= first);
 
        ::free(p);
        lpt::process::memory third;
        std::cout << "Mem diff: " << third.total() - first.total() << std::endl;
        assert(third == first);
    }
    {
        std::string s("abc");
        lpt::process::memory second;
        std::cout << "Mem diff: " << second.total() - first.total() << std::endl;
        assert(second > first);
    }
 
    lpt::process::memory fourth;
    assert(first == fourth);
 
    return 0;
}

