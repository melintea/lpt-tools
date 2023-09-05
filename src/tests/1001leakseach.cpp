/*
 * Copyright 2012 Aurelian Melinte. 
 * Released under GPL 3.0 or later. 
 *
 * Test for memory leaks reports. 
 *
 * $Id: main.cpp,v 1.3 2011/09/10 23:54:49 amelinte Exp amelinte $
 *
 *  
 */
 
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <algorithm>

#include <cstdlib>
#include <cassert>

#include <mcheck.h>
#include <sys/mman.h>
#include <unistd.h>

#include <pthread.h>


void
leaks(void)
{
    const int num_leaks = 1001;

    char *p = 0;
    std::string *ps = 0;

    for (int i=0; i < num_leaks; i++) {
        p = (char*)malloc(101);
        //std::cout << std::hex << p << "\n";
    }
    free(p);
    mtrace(); //report

    for (int i=0; i < num_leaks; i++) {
        p = new char[202];
        //std::cout << std::hex << p << "\n";
    }
    delete[] p;
    mtrace(); //report

    for (int i=0; i < num_leaks; i++) {
        ps = new std::string("leak");
    }
    delete ps;
    mtrace(); //report

    for (int i=0; i < num_leaks; i++) {
        p = (char*)mmap(0, 303, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        //std::cout << std::hex << p << "\n";
    }
    munmap(p, 303);
    mtrace(); //report
}

void
func3(void)
{
    leaks();
}

void
func2(void)
{
    func3();
}

bool
tests(void)
{
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    ::sleep(2);
    func2();
    return true;
}

void * 
pwrapper(void *)
{
    std::cout << "Thread " << pthread_self() << std::endl;
    tests();
    return 0;
}

int 
main(void)
{
    mtrace(); //start the engine

    const int num_threads = 5;
    
    /* 4.7 this barfs:
    {
        std::vector<std::future<bool> > workers;
    
        for (int i = 0; i < num_threads; ++i) {
            auto worker = std::async([] {return tests();});
            workers.push_back(std::move(worker));
        }
    
        //Wait for completion
        for (auto& w: workers) {
            w.get(); //barfs here
        }
    }
    */
    
    {
        pthread_t ts[num_threads] = {0};
        for (int i = 0; i < num_threads; ++i) {
            int rc = pthread_create(&ts[i], nullptr, pwrapper, nullptr);
            assert(rc == 0);
        }
        for (int i = 0; i < num_threads; ++i) {
            void *status = 0;
            int rc = pthread_join(ts[i], &status);
            assert(rc == 0);
        }
    }
    
    // Let the interposition lib file a final report 
    //muntrace();
    
    std::cout << "**** 1001leaks will exit ****" << std::endl;
    return 0;
}
