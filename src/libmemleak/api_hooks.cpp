/*
 *  $Id: mtrace.cpp,v 1.4 2011/09/10 23:54:49 amelinte Exp amelinte $
 *
 *  Copyright 2011 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  Hooking memory allocation through mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
 */

/* More debugging hooks for `malloc'.
   Copyright (C) 1991-1994,1996-2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
             Written April 2, 1991 by John Gilmore of Cygnus Support.
             Based on mcheck.c by Mike Haertel.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <malloc.h>
#include <mcheck.h>

#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h> 

#include <signal.h>

#include <exception>
#include <iostream>
#include <fstream>
#include <mutex>
#include <atomic>

#include <lpt/callstack/call_stack.hpp>

#include "report.h" 
#include "config.h"


/*
 * Globals not constructed when interposition lib init called? Must allocate. 
 */ 
static std::ofstream *_mallstream;

static std::mutex _hooks_lock;


/* Address to breakpoint on accesses to... */
__ptr_t mallwatch;



void *__hlibc = nullptr;
int (*__libc_munmap)(void *addr, size_t length) = nullptr;
void* (*__libc_mmap)(void *addr, size_t length, int  prot, int flags, int fd, off_t offset) = nullptr;


bool _capture_on = false; // mtrace()/muntrace() calls



static inline void 
serror(__ptr_t ptr, const char *msg, const char *file, int line)
{
    //lpt::stack::libc::basic_dump(msg);
}

// For debugging
static inline void mdebug(const char* msg, const void* ptr)
{
    //std::cerr << msg << std::hex << ptr << std::dec << std::endl;
}



/* This function is called when the block being alloc'd, realloc'd, or
   freed has an address matching the variable "mallwatch".  In a debugger,
   set "mallwatch" to the address of interest, then put a breakpoint on
   tr_break.  */

extern "C"
void
tr_break ()
{
}


static inline void
dump_stack()
{
    lpt::stack::default_stack here(true);
    *_mallstream << "**[ Stack: " << here.depth() << "\n"
                 << lpt::stack::default_call_stack_info(here) << "**] Stack\n\n" << std::flush;
}


//extern "C" void *__mmap(void *addr, size_t length, int  prot, int flags, int fd, off_t offset);
extern "C" void *mmap(void *addr, size_t length, int  prot, int flags, int fd, off_t offset)
{
    void *palloc = __libc_mmap(addr, length, prot, flags, fd, offset);
    
    if (_capture_on) {
        libmemleak::alloc(palloc, length);
    }
        
    //printf("mmap %p\n", palloc);
    
    return palloc;
}


//extern "C" int __munmap(void *addr, size_t length); 
extern "C" int munmap(void *addr, size_t length)
{
    int ret = __libc_munmap(addr, length);
        
    if (_capture_on) {
        libmemleak::free(addr, length);
    }
    
    //printf("munmap %p\n", addr);
    
    return ret;
}


extern "C" void *__libc_malloc(size_t size); 
extern "C" void *malloc(size_t size)
{
    void *ptr = __libc_malloc(size);
    mdebug("M ", ptr);
    
    if ( _capture_on) {
            libmemleak::alloc(ptr, size);
    }
    
    return ptr;
}


extern "C" void __libc_free(void *ptr);
extern "C" void free(void *ptr)
{
    __libc_free(ptr);
    mdebug("F ", ptr);

    if (_capture_on) {
            libmemleak::free(ptr, 0);
    }
        else {
            serror(ptr, "Untraced free", __FILE__, __LINE__);
        }
}


extern "C" void *__libc_realloc(void *ptr, size_t size);
extern "C" void *realloc(void *ptr, size_t size)
{
    void *prealloc = __libc_realloc(ptr, size);
    mdebug("R ", prealloc);

    if (_capture_on) {
            if (prealloc == nullptr) {
            // Failed realloc.  
            libmemleak::error(ptr, size);
            } else if (ptr == nullptr) {
            libmemleak::alloc(prealloc, size);
            } else {
                libmemleak::realloc(ptr, //old 
                                    prealloc, //new
                                    size);
            }
    }
        else {
            serror(prealloc, "Untraced realloc", __FILE__, __LINE__);
        }

    return prealloc;
}


extern "C" void *__libc_memalign(size_t boundary, size_t size);
extern "C" void *memalign(size_t boundary, size_t size)
{
    void *ptr = __libc_memalign(boundary, size);
    mdebug("A ", ptr);

    if (_capture_on) {
             libmemleak::alloc(ptr, size);
    }
        else {
            serror(ptr, "Untraced memalign", __FILE__, __LINE__);
        }

    return ptr;
}



void
resolve_hooks()
{
    if (__libc_mmap == nullptr) {
        __hlibc = dlopen("libc.so.6", RTLD_LAZY|RTLD_GLOBAL);
        __libc_mmap = (void* (*)(void*, size_t, int, int, int, off_t))dlsym(__hlibc, "mmap");
        __libc_munmap = (int (*)(void*, size_t))dlsym(__hlibc, "munmap");
    }
}


extern "C" void
mtrace ()
{
    //std::cout << "mtrace() substitute\n";

    // Make sure not to track memory when globals get destructed
    static std::atomic<bool> _atexit(false);
    if (!_atexit.load(std::memory_order_acquire)) {
        int ret = atexit(muntrace);
        assert(0 == ret);
        _atexit.store(true, std::memory_order_release);
    }

    if (!_capture_on) {
        _capture_on = true;
    }
    else {
        libmemleak::report();
    }
}

extern "C" void
muntrace ()
{
    //std::cout << "mUNtrace() substitute\n" << std::flush;

    _capture_on = false;

    libmemleak::report();
}

