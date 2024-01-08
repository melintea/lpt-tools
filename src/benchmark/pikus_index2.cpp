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
constexpr unsigned int L = 1 << 16;  // str length
constexpr unsigned int N = 1 << 11;  // number of (sub)strings to compare

std::unique_ptr<char[]>  s(new char[L]);
std::vector<const char*> vs(N);


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
bool compare(const char* s1, const char* s2, IDX_T l) 
{
    for (IDX_T i1 = 0, i2 = 0; i1 < l; ++i1, ++i2) {
        if (s1[i1] != s2[i2]) return s1[i1] > s2[i2];
    }
    return false;
}

// Not a test, juts prepare data
void BM_prepare(benchmark::State& state) 
{
    std::minstd_rand rgen;
    ::memset(s.get(), 'a', L*sizeof(char));
    for (unsigned int i = 0; i < L/1024; ++i) {
        s[rgen() % (L - 1)] = 'a' + (rgen() % ('z' - 'a' + 1));
    }
    s[L-1] = 0;
    for (unsigned int i = 0; i < N; ++i) {
        vs[i] = &s[rgen() % (L - 1)];
    }

    int dummy = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(++dummy);
    }
}
BENCHMARK(BM_prepare);

template <typename IDX_T>
void BM_loop(benchmark::State& state) {
    
    auto vss = vs;
    
    for (auto _ : state) {
        std::sort(vss.begin(), vss.end(), [&](const char* a, const char* b) {return compare<IDX_T>(a, b, L); });
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_loop<int>);
BENCHMARK(BM_loop<unsigned int>);
BENCHMARK(BM_loop<size_t>);


BENCHMARK_MAIN();
