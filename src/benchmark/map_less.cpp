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
#include <unordered_map>
#include <vector>


//-----------------------------------------------------------------------------
constexpr unsigned int L = 1 << 16;  // str length
constexpr unsigned int N = 1 << 11;  // number of (sub)strings in map

std::unique_ptr<char[]>  s(new char[L]);
std::map<std::string, std::string>               plainMap;
std::map<std::string, std::string, std::less<>>  lessMap;

std::unordered_map<std::string, std::string>     plainUmap;
struct transparent_stringview_hash : std::hash<std::string_view> {
    using is_transparent = void;
};
std::unordered_map<std::string, std::string, transparent_stringview_hash, std::equal_to<>> lessUmap;


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
        plainMap[ps]  = ps;
        lessMap[ps]   = ps;
        plainUmap[ps] = ps;
        lessUmap[ps]  = ps;
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


template <typename FIND_T>
void BM_Uplain(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(L); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( plainUmap.find(FIND_T(&s[i])) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_Uplain<const char*>);
BENCHMARK(BM_Uplain<std::string>);
BENCHMARK(BM_Uplain<const char*>);
//BENCHMARK(BM_Uplain<std::string_view>);


template <typename FIND_T>
void BM_Uless(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(L); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( lessUmap.find(FIND_T(&s[i])) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_Uless<const char*>);
BENCHMARK(BM_Uless<std::string>);
BENCHMARK(BM_Uless<const char*>);
BENCHMARK(BM_Uless<std::string_view>);


BENCHMARK_MAIN();
