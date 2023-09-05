/*
 *  $Id: mtrace.cpp,v 1.4 2011/09/10 23:54:49 amelinte Exp amelinte $
 *
 *  Copyright 2011 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
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
#include <pthread.h>

#include <signal.h>

#include <exception>
#include <iostream>
#include <fstream>
#include <mutex>

#include <lpt/callstack/call_stack.hpp>


/*
 * Globals not constructed when interposition lib init called? Must allocate. 
 */ 
static std::ofstream *_mallstream;
static const char _mallenv[]= "MALLOC_TRACE";

static std::mutex _hooks_lock;


/*
 * This mechanism is not foolproof. Works on some platforms but will crash on some.
 * On amd64/gcc4.7.2/Debian Wheezy TLS will trip over the memalign hook before the flag
 * being set and this will result in an infinite loop:
 *    ...etc...
 *    #157230 0x00007ffff7bd900e in tr_memalignhook (alignment=1, size=1, caller=0x7ffff7dedbf0) at libmtrace/mtrace.cpp:317
 *    #157231 0x00007ffff7dedbf0 in ?? () from /lib64/ld-linux-x86-64.so.2
 *    #157232 0x00007ffff7dee110 in __tls_get_addr () from /lib64/ld-linux-x86-64.so.2
 *    #157233 0x00007ffff7bd900e in tr_memalignhook (alignment=1, size=1, caller=0x7ffff7dedbf0) at libmtrace/mtrace.cpp:317
 *    #157234 0x00007ffff7dedbf0 in ?? () from /lib64/ld-linux-x86-64.so.2
 *    #157235 0x00007ffff7dee110 in __tls_get_addr () from /lib64/ld-linux-x86-64.so.2
 *    #157236 0x00007ffff7bd8b33 in tr_mallochook (size=31, caller=0x7ffff772c07d) at libmtrace/mtrace.cpp:213
 *    #157237 0x00007ffff772c07d in operator new(unsigned long) () from /usr/lib/x86_64-linux-gnu/libstdc++.so.6
 */
/* 
 * Demangling functions are calling malloc(). Do not trace these allocations
 * once we enter our hooks (infinite loop). 
 */
#ifndef __cplusplus
typedef enum {false, true} bool;
#endif
static volatile __thread bool _in_trace = false;


#define STACK_DEPTH         (40)


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


static inline void 
serror(__ptr_t ptr, const char *msg, const char *file, int line)
{
    //lpt::stack::libc::basic_dump(msg);
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


inline static int 
mbacktrace(void **buffer, int size)
{
    return backtrace(buffer, size);
}


static inline void
dump_stack()
{
    lpt::stack::call_stack<40> here(true);
    *_mallstream << "**[ Stack: " << here.depth() << "\n"
                 << lpt::stack::call_stack_info<lpt::stack::call_stack<40> >(here) << "**] Stack\n\n" << std::flush;
}


static void
trace_where (const __ptr_t caller)
{
    assert(_in_trace == true);
  
    if (caller != nullptr)
    {
        // "@ /usr/lib/libstdc++.so.6:(_Znwj+27)[0x400ff727] + 0x8bf8998 0x13"
        // "@ [%p] "
        lpt::stack::basic_symbol_info caller_frame(caller);
        *_mallstream << "@ " << caller_frame.binary_file()
                     << ":(" << caller_frame.demangled_function_name() 
                     << (caller_frame.delta() > 0 ? '+' : '-') << caller_frame.delta() << ")"
                     << "[" << std::hex << caller_frame.addr() << "] "
                     ;
    }
}

static inline void 
trace_what(const char * format,
           __ptr_t ptr,
           unsigned long long size)
{
    *_mallstream << format << " " << std::hex << ptr << " " << size << std::endl;

    dump_stack();
}

static void
tr_freehook (__ptr_t ptr,
     const __ptr_t caller)
{
    if (ptr == nullptr)
        return;
    
    /* The alloc/free operation is a result of the API calls we make to resolve
     * symbols while in a mem trace on this thread.  Do not log this.
     */
    bool oldval = __sync_val_compare_and_swap(&_in_trace, false, true);
    if (false == oldval)
    {
        assert(oldval == false && _in_trace == true);
        
        trace_where (caller);
        /* Be sure to print it first.  */
        trace_what ("-", ptr, 0/*ignored*/);
        
        oldval = __sync_val_compare_and_swap(&_in_trace, true, false);
        assert(oldval == true && _in_trace == false);
    }
    else {
        serror(ptr, "Untraced free", __FILE__, __LINE__);
    }

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

    bool oldval = __sync_val_compare_and_swap(&_in_trace, false, true);
    if (false == oldval)
    {
        assert(oldval == false && _in_trace == true);
        
        trace_where (caller);
        /* We could be printing a NULL here; that's OK.  */
        trace_what ("+", hdr, (unsigned long int) size);
        
        oldval = __sync_val_compare_and_swap(&_in_trace, true, false);
        assert(oldval == true && _in_trace == false);
    }
    else {
        serror(hdr, "Untraced malloc", __FILE__, __LINE__);
    }

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

    bool oldval = __sync_val_compare_and_swap(&_in_trace, false, true);
    if (false == oldval)
    {
        assert(oldval == false && _in_trace == true);

        trace_where (caller);
        if (hdr == nullptr) {
            /* Failed realloc.  */
            trace_what ("!", ptr, (unsigned long int) size);
        } else if (ptr == nullptr) {
            trace_what ("+", hdr, (unsigned long int) size);
        } else {
            trace_what ("<", ptr, 0/*ignored*/);
            trace_where (caller);
            trace_what (">", hdr, (unsigned long int) size);
        }

        oldval = __sync_val_compare_and_swap(&_in_trace, true, false);
        assert(oldval == true && _in_trace == false);
    }
    else {
        serror(ptr, "Untraced realloc", __FILE__, __LINE__);
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

    bool oldval = __sync_val_compare_and_swap(&_in_trace, false, true);
    if (false == oldval)
    {
        assert(oldval == false && _in_trace == true);

        trace_where (caller);
        /* We could be printing a NULL here; that's OK.  */
        trace_what ("+", hdr, (unsigned long int) size);

        oldval = __sync_val_compare_and_swap(&_in_trace, true, false);
        assert(oldval == true && _in_trace == false);
    }
    else {
        serror(hdr, "Untraced memalign", __FILE__, __LINE__);
    }

    if (hdr == mallwatch) {
        tr_break ();
    } 

    return hdr;
}



#ifdef _LIBC

/* This function gets called to make sure all memory the library
   allocates get freed and so does not irritate the user when studying
   the mtrace output.  */
static void __libc_freeres_fn_section
release_libc_mem (void)
{
    /* Only call the free function if we still are running in mtrace mode.  */
    if (_mallstream && _mallstream->is_open())
      __libc_freeres ();
}
#endif


/* We enable tracing if either the environment variable MALLOC_TRACE
   is set, or if the variable mallwatch has been patched to an address
   that the debugging user wants us to stop on.  When patching mallwatch,
   don't forget to set a breakpoint on tr_break!  */

extern "C" void
mtrace ()
{
    //std::cout << "mtrace() substitute\n";

    std::unique_lock<std::mutex> lock(_hooks_lock);

    char *mallfile = 0;

    /* Don't panic if we're called more than once.  */
    if (_mallstream && _mallstream->is_open())
        return;

    /*
     * Call backtrace before hooking because dynamically linked backtrace() will call malloc().
     * Otherwise we run in an infinite loop. 
     */
    void *stack_buf[STACK_DEPTH] = {0};
    int  n = mbacktrace(stack_buf, STACK_DEPTH);
    n = n; //Shut compiler warning

    mallfile = getenv (_mallenv);
    if (mallfile != nullptr || mallwatch != nullptr)
    {
        _mallstream = new std::ofstream(mallfile != nullptr ? mallfile : "/dev/null");
        if (_mallstream->is_open())
        {
            *_mallstream << "= Start\n";

            _tr_old_free_hook = __free_hook;
            __free_hook = tr_freehook;
            _tr_old_malloc_hook = __malloc_hook;
            __malloc_hook = tr_mallochook;
            _tr_old_realloc_hook = __realloc_hook;
            __realloc_hook = tr_reallochook;
            _tr_old_memalign_hook = __memalign_hook;
            __memalign_hook = tr_memalignhook;
        }
    }
}

extern "C" void
muntrace ()
{
    //std::count << "mUNtrace() substitute\n";

    //std::unique_lock<std::mutex> lock(_hooks_lock);

    if (!_mallstream || !_mallstream->is_open())
      return;

    *_mallstream << "= End\n";
    _mallstream->close();
    delete _mallstream; 
    _mallstream = nullptr;

    __free_hook = _tr_old_free_hook;
    __malloc_hook = _tr_old_malloc_hook;
    __realloc_hook = _tr_old_realloc_hook;
    __memalign_hook = _tr_old_memalign_hook;
}
