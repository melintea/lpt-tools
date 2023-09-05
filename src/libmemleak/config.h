/*
 *  $Id: libmtrace.cpp,v 1.1 2011/09/05 21:29:22 amelinte Exp amelinte $
 *
 *  
 */

#pragma once 


#define VERSION "1.0"


typedef struct _mcfg {
    const char * _mtrace_file;
    const char * _config;
    bool         _mtrace_init; // Call mtrace() at lib init time
} mcfg;

extern mcfg _mconfig;
