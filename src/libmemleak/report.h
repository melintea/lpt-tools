/*
 *
 */

#include <lpt/mem_stats.hpp> //memsize_type

#pragma once


namespace libmemleak {

void init();
void fini();

void alloc(__ptr_t ptr, memsize_type size);
void free(__ptr_t ptr, memsize_type size);
void realloc(__ptr_t newptr, __ptr_t oldptr, memsize_type size);
void allign(__ptr_t ptr, memsize_type size);

void error(__ptr_t ptr, memsize_type size);

void report();

} //namespace
