/*
 *  Copyright 2012 Aurelian Melinte. 
 *  Released under GPL 3.0 or later. 
 *
 *  You need a C++0x compiler. 
 *
 *  \brief Enforce on-stack instantiation
 * 
 */
 
 
#pragma once 

namespace lpt { 

class stackonly
{
protected:
   
    static void* operator new (size_t size);
    static void* operator new (size_t size, void* mem);

};
  
} //namespace
