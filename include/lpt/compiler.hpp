/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Various instructions for the compiler to be used from within code.
 */

#pragma once 

namespace lpt { namespace intel {

/*
 * p can be used to access all memory - placate optimizer for p
 * Usage:
 *   disable_optimizer(v.data());
 *   v.push_back(xyz);
 *   barrier();
 */
inline void disable_optimizer(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

// Pretend all memory was read & written
//#define barrier() __asm__ __volatile__("": : :"memory")
inline void barrier() {
    asm volatile("" : : : "memory");
}

#define EMBEDDED_BREAKPOINT  asm volatile ("int3;")
//#define EMBEDDED_BREAKPOINT               \
//    asm("0:"                              \
//        ".pushsection embed-breakpoints;" \
//        ".quad 0b;"                       \
//        ".popsection;")

/* 
 * Placate optimizer for reading x; from kernel code
 * Usage: owner = ACCESS_ONCE(lock->owner);
 */
#ifndef ACCESS_ONCE
#  define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#else
#  warning ACCESS_ONCE already defined
#endif

#ifndef ACCESSB
#  define ACCESSB(x)     (*(volatile bool *)&(x))
#else
#  warning ACCESSB already defined
#endif

}} //namespace lpt::intel


