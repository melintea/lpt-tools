/*
 *  $Id: libmtrace.cpp,v 1.1 2011/09/05 21:29:22 amelinte Exp amelinte $
 *
 *  Copyright 2011 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  mtrace()/muntrace() & tools.  Based on glibc 2.6.1 and binutils 2.18. 
 */

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <mcheck.h>



void 
libmtrace_atexit(void)
{
    muntrace();
}


#ifdef __cpluslpus
#  extern "C" 
#endif
void _init()  __attribute__((constructor));

#ifdef __cpluslpus
#  extern "C" 
#endif
void 
_init()
{
    atexit(libmtrace_atexit);
    mtrace();
}


#ifdef __cpluslpus
#  extern "C" 
#endif
void  _fini()  __attribute__((destructor)); 

#ifdef __cpluslpus
#  extern "C" 
#endif
void 
_fini()
{
}

