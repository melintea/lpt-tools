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

#include <lpt/call_stack.hpp>

#include "report.h" 
#include "config.h"


/*
 * Globals not constructed when interposition lib init called? Must allocate. 
 */ 
static std::ofstream *_mallstream;

static std::mutex _hooks_lock;


/* Address to breakpoint on accesses to... */
__ptr_t mallwatch;


/* Old hook values.  */
static void (*_tr_old_free_hook) (__ptr_t ptr, const __ptr_t);
static __ptr_t (*_tr_old_malloc_hook) (__malloc_size_t size, const __ptr_t);
static __ptr_t (*_tr_old_realloc_hook) (__ptr_t ptr, __malloc_size_t size,
                                        const __ptr_t);
static __ptr_t (*_tr_old_memalign_hook) (__malloc_size_t __alignment,
                                         __malloc_size_t __size,
                                         __const __ptr_t);


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


static void
tr_freehook (__ptr_t ptr,
     const __ptr_t caller)
{
    if (ptr == nullptr)
        return;
    
    mdebug("f ", ptr);

    libmemleak::free(ptr, 0);

    if (ptr == mallwatch)
        tr_break();

    {
        std::unique_lock<std::mutex> lock(_hooks_lock);

        __free_hook = _tr_old_free_hook;
        if (_tr_old_free_hook != nullptr) {
            (*_tr_old_free_hook) (ptr, caller);
        } else {
            free (ptr);
        }
        __free_hook = tr_freehook;
    }
}

static __ptr_t
tr_mallochook (__malloc_size_t size,
     const __ptr_t caller)
{
    __ptr_t hdr = 0;

    {
        std::unique_lock<std::mutex> lock(_hooks_lock);

        __malloc_hook = _tr_old_malloc_hook;
        if (_tr_old_malloc_hook != nullptr) {
            hdr = (__ptr_t) (*_tr_old_malloc_hook) (size, caller);
        } else {
            hdr = (__ptr_t) malloc (size);
        }
        __malloc_hook = tr_mallochook;
    }
    mdebug("m ", hdr);

    libmemleak::alloc(hdr, size);
        
    if (hdr == mallwatch)
        tr_break ();

    return hdr;
}

static __ptr_t
tr_reallochook (
     __ptr_t ptr,
     __malloc_size_t size,
     const __ptr_t caller)
{
    __ptr_t hdr;

    if (ptr == mallwatch)
        tr_break ();

    mdebug("r ", ptr);
    {
        std::unique_lock<std::mutex> lock(_hooks_lock);

        __free_hook = _tr_old_free_hook;
        __malloc_hook = _tr_old_malloc_hook;
        __realloc_hook = _tr_old_realloc_hook;
        if (_tr_old_realloc_hook != nullptr) {
            hdr = (__ptr_t) (*_tr_old_realloc_hook) (ptr, size, caller);
        } else {
            hdr = (__ptr_t) realloc (ptr, size);
        }
        __free_hook = tr_freehook;
        __malloc_hook = tr_mallochook;
        __realloc_hook = tr_reallochook;
    }

    if (hdr == nullptr) {
        /* Failed realloc.  */
        libmemleak::error(ptr, size);
    } else if (ptr == nullptr) {
        libmemleak::alloc(hdr, size);
    } else {
        libmemleak::realloc(ptr/*old*/, hdr/*new*/, size);
    }

    if (hdr == mallwatch) {
        tr_break ();
    }

    return hdr;
}

static __ptr_t
tr_memalignhook (
     __malloc_size_t alignment, 
     __malloc_size_t size,
     const __ptr_t caller)
{
    __ptr_t hdr;

    {
        std::unique_lock<std::mutex> lock(_hooks_lock);

        __memalign_hook = _tr_old_memalign_hook;
        __malloc_hook = _tr_old_malloc_hook;
 
        if (_tr_old_memalign_hook != nullptr) {
            hdr = (__ptr_t) (*_tr_old_memalign_hook) (alignment, size, caller);
        } else {
            hdr = (__ptr_t) memalign (alignment, size);
        }

        __memalign_hook = tr_memalignhook;
        __malloc_hook = tr_mallochook;
    }
    mdebug("a ", hdr);

    libmemleak::allign(hdr, size);

    if (hdr == mallwatch) {
        tr_break ();
    } 

    return hdr;
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
        std::unique_lock<std::mutex> lock(_hooks_lock);

        _tr_old_free_hook = __free_hook;
        __free_hook = tr_freehook;
        _tr_old_malloc_hook = __malloc_hook;
        __malloc_hook = tr_mallochook;
        _tr_old_realloc_hook = __realloc_hook;
        __realloc_hook = tr_reallochook;
        _tr_old_memalign_hook = __memalign_hook;
        __memalign_hook = tr_memalignhook;
    
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

    //std::unique_lock<std::mutex> lock(_hooks_lock);

    __free_hook = _tr_old_free_hook;
    __malloc_hook = _tr_old_malloc_hook;
    __realloc_hook = _tr_old_realloc_hook;
    __memalign_hook = _tr_old_memalign_hook;
    
    _capture_on = false;

    libmemleak::report();
}

