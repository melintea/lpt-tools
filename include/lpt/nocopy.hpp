/*
 *  Copyright 2012 Aurelian Melinte. 
 *  Released under GPL 3.0 or later. 
 *
 *  You need a C++0x compiler. 
 *
 *  \brief prevent copies
 * 
 */
 
 
#pragma once 

namespace lpt { 

struct nocopy
{
    nocopy()  = default;
    ~nocopy() = default;
      
    nocopy( const nocopy& ) = delete;
    const nocopy& operator=( const nocopy& ) = delete;
};
  

struct immovable
{
    nocopy( nocopy&& ) = delete;
    nocopy& operator=( nocopy&& ) = delete;
};

  
} //namespace
