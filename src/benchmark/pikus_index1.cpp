/*
 * Which index is faster in a for() loop: int
 */
 
#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <vector>


//-----------------------------------------------------------------------------
//constexpr unsigned int L = 1 << 18; // lenght
//constexpr unsigned int N = 1 << 14; // number of (sub)strings to compare

#define ARGS \
    ->Arg(1<<20)

template <typename IDX_T>
bool compare(const char* s1, const char* s2) 
{
    for (IDX_T i1 = 0, i2 = 0; ; ++i1, ++i2) {
        if (s1[i1] != s2[i2]) {
	    return (s1[i1] > s2[i2]);
	}
    }
    return false;
}

template <typename IDX_T>
bool compare(const char* s1, const char* s2, IDX_T l) {
    for (IDX_T i1 = 0, i2 = 0; i1 < l; ++i1, ++i2) {
        if (s1[i1] != s2[i2]) return s1[i1] > s2[i2];
    }
    return false;
}

void BM_loop_int(benchmark::State& state) {
    int N = state.range(0);
    std::unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare<int>(s1, s2, N));
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_loop_int) ARGS;

void BM_loop_size_t(benchmark::State& state) {
    size_t N = state.range(0);
    std::unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare<size_t>(s1, s2, N));
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_loop_size_t) ARGS;

void BM_loop_uint(benchmark::State& state) {
    const unsigned int N = state.range(0);
    std::unique_ptr<char[]> s(new char[2*N]);
    ::memset(s.get(), 'a', 2*N*sizeof(char));
    s[2*N-1] = 0;
    const char* s1 = s.get(), *s2 = s1 + N;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(compare<unsigned int>(s1, s2, N));
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_loop_uint) ARGS;

BENCHMARK_MAIN();
