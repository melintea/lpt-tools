/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  gperftools.
 *  https://github.com/gperftools/gperftools
 *
 */

#ifndef INCLUDED_gperf_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5
#define INCLUDED_gperf_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5

#pragma once

#include <gperftools/profiler.h>

namespace lpt { namespace gperftools {

inline void begin() { __asm volatile("# LLVM-MCA-BEGIN"); }
inline void end()   { __asm volatile("# LLVM-MCA-END");   }

/*
 *
 */
class profiler
{
public:

    profiler(const char* fileName)  
    { 
        ProfilerStart(fileName); 
    }
    
    profiler() 
    { 
        //TODO: should be a sigleton?
        ProfilerFlush();
	ProfilerStop();   
    }

    profiler( const profiler& other )            = delete;
    profiler& operator=( const profiler& other ) = delete;

    profiler( profiler&& other )                 = delete;
    profiler& operator=( profiler&& other )      = delete;

}; // profiler

}} //namespace lpt::gperftools


#endif //#define INCLUDED_gperf_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5
