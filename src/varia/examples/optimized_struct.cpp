/*
 * Verify if struct elements are declared by decreasing size
 */

#include <lpt/cpu.hpp>
 
#include <cstdlib>


// Valid: Sorted from largest (8 bytes) to smallest (1 byte)
struct OptimizedStruct 
{
    double d; // 8 bytes
    int i;    // 4 bytes
    char c;   // 1 byte
};

// Invalid: Unsorted (4 bytes, then 8 bytes, then 1 byte)
struct BadStruct 
{
    int i;    // 4 bytes
    double d; // 8 bytes
    char c;   // 1 byte
};


int main()
{
    lpt::cpu::is_cache_optimal<OptimizedStruct>();
    lpt::cpu::is_cache_optimal<BadStruct>();
    return EXIT_SUCCESS;
}

