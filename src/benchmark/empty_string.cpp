/*
 * empty string move vs copy
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
#include <string>
#include <unordered_map>
#include <vector>


//-----------------------------------------------------------------------------
constexpr unsigned int L = 1 << 16;  // str length
constexpr unsigned int N = 1 << 11;  // number of (sub)strings in map

const std::string empty;

void BM_str_copy(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
	    std::string s = empty;
            benchmark::DoNotOptimize( s );
	    benchmark::ClobberMemory();
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
void BM_str_move(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
	    std::string s = std::move<std::string>({});
            benchmark::DoNotOptimize( s );
	    benchmark::ClobberMemory();
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_str_copy);
BENCHMARK(BM_str_move);
BENCHMARK(BM_str_copy);
BENCHMARK(BM_str_move);
BENCHMARK(BM_str_copy);
BENCHMARK(BM_str_move);
BENCHMARK(BM_str_copy);
BENCHMARK(BM_str_move);


BENCHMARK_MAIN();
