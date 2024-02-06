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

/*
 *
 */
class cpu_profiler
{
public:

    cpu_profiler(const char* fileName)  
    { 
        ProfilerStart(fileName); 
    }
    
    profiler() 
    { 
        //TODO: should be a sigleton?
        ProfilerFlush();
	ProfilerStop();   
    }

    cpu_profiler( const cpu_profiler& other )            = delete;
    cpu_profiler& operator=( const cpu_profiler& other ) = delete;

    cpu_profiler( cpu_profiler&& other )                 = delete;
    cpu_profiler& operator=( cpu_profiler&& other )      = delete;

}; // cpu_profiler

}} //namespace lpt::gperftools


#endif //#define INCLUDED_gperf_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5
