/*
 *  $Id: libmtrace.cpp,v 1.1 2011/09/05 21:29:22 amelinte Exp amelinte $
 *
 *  Copyright 2011 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  mtrace()/muntrace() & tools.  Based on glibc 2.6.1 and binutils 2.18. 
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <mcheck.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include "config.h"
#include "report.h"


mcfg _mconfig = { _mtrace_file: nullptr,
                  _config:  nullptr,
                  _mtrace_init: false, 
                };


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
    extern void resolve_hooks();
 
    _mconfig._mtrace_file = getenv ("MALLOC_TRACE");
    _mconfig._config = getenv ("MEMLEAK_CONFIG");

    if (_mconfig._config) {
        if (strcasestr(_mconfig._config, "mtraceinit")) {
            _mconfig._mtrace_init = true;
        }
    }
    
              
    // Ensure the sigletons are instantiated before atexit()
    // They will be used atexit by muntrace().
    libmemleak::init();
    
    int ret = atexit(libmtrace_atexit);
    assert(0 == ret);

    resolve_hooks();
    
    if (_mconfig._mtrace_init) {
        mtrace();
    }
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
    //std::cout << "_fini\n" << std::flush;
    libmemleak::fini();
}

