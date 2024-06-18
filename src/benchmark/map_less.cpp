/*
 * Which index is faster in a for() loop: int
 */
 
#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <vector>


//-----------------------------------------------------------------------------
constexpr unsigned int L = 1 << 16;  // str length
constexpr unsigned int N = 1 << 11;  // number of (sub)strings in map

std::unique_ptr<char[]>  s(new char[L]);
std::map<std::string, std::string>               plainMap;
std::map<std::string, std::string, std::less<>>  lessMap;

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
        const char* ps = &s[rgen() % (L - 1)];
        plainMap[ps] = ps;
        lessMap[ps]  = ps;
    }

    int dummy = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(++dummy);
    }
}
BENCHMARK(BM_prepare);

template <typename FIND_T>
void BM_plain(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(L); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( plainMap.find(FIND_T(&s[i])) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_plain<const char*>);
BENCHMARK(BM_plain<std::string>);
BENCHMARK(BM_plain<const char*>);
//BENCHMARK(BM_plain<std::string_view>);

template <typename FIND_T>
void BM_less(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(L); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( lessMap.find(FIND_T(&s[i])) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_less<const char*>);
BENCHMARK(BM_less<std::string>);
BENCHMARK(BM_less<const char*>);
BENCHMARK(BM_less<std::string_view>);


BENCHMARK_MAIN();
