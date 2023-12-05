/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  LLVM MCA test.
 */


 #include <lpt/compiler.hpp>
 #include <lpt/llvm_mca.hpp>
 
 #include <iostream>
 #include <vector>
 
 #include <stdlib.h>
 
 int main()
 {
 
    ::srand(1);
    constexpr const unsigned int N = 2048;
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = ::rand();
        v2[i] = ::rand();
        c1[i] = ::rand() >= 0;
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    
    unsigned long a1 = 0, a2 = 0;
    for (size_t i = 0; i < N; ++i) {
        //lpt::llvm::begin();
	{
	    lpt::llvm::mca m;
            a1 += b1[i] ? p1[i] : p2[i];
	}
        //lpt::llvm::end();
    }
    lpt::intel::disable_optimizer(&a1);
    lpt::intel::disable_optimizer(&a2);
    lpt::intel::barrier();

    return EXIT_SUCCESS;
 }
