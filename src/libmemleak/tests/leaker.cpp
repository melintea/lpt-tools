/*
 * Copyright 2012 Aurelian Melinte. 
 * Released under GPL 3.0 or later. 
 *
 * Test for mtrace()/muntrace() with stack dump. 
 *
 * $Id: main.cpp,v 1.3 2011/09/10 23:54:49 amelinte Exp amelinte $
 *
 *  
 */

#include <unistd.h>

#include <iostream>

#include <cstdlib>
#include <mcheck.h>
#include <dlfcn.h>

int 
main(void)
{
    //sleep(20);

    // Usage: 
    // export MALLOC_TRACE=./mtrace.log
    // mtrace ./a.out mtrace.log
    //mtrace();

    std::string str("noleak");
    
    std::string* pstr = new std::string("leak");
    pstr = pstr; //shut compiler
    
    char *leak = (char*)malloc(1024); leak=leak;
    char *nope = (char*)malloc(2048);
    free(nope);
    
    // Calling muntrace() here will report 'str' as a leak 
    //muntrace();
    
    std::cout << "**** dleaker will exit ****" << std::endl;
    return 0;
}
