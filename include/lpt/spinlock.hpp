/*
 *  Copyright 2012 Aurelian Melinte. 
 *  Released under GPL 3.0 or later. 
 *
 *  You need a C++0x compiler. 
 * 
 *  Fedor Pikus spinlock.
 * 
 */
 
#include <atomic>

#include <time.h>

#pragma once 


namespace lpt { 

class spinlock
{

public:

    spinlock() : _flag(0) {}
    ~spinlock() = default;
    
    spinlock(spinlock&& other) noexcept                 = default;
    spinlock& operator=(spinlock&& other) noexcept      = default;
    
    spinlock(const spinlock& error) noexcept            = delete;
    spinlock& operator=(const spinlock& other) noexcept = delete;
    
    void lock()
    {
        static const timespec ns = {0, 1};
	for (int i = 0;
	     _flag.load(std::memory_order_relaxed) || _flag.exchange(1, std::memory_order_acquire);
	     ++i) {
	     if (i == 8) {
	         i = 0;
		 ::nanosleep(&ns, NULL);
	     }
	}
    }
    
    void unlock()
    {
        _flag.store(0, std::memory_order_release);
    }

private:

    std::atomic<unsigned int> _flag;

};

} //namespace
