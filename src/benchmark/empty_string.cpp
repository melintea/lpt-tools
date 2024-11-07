/*
 * empty string move vs copy
 * - move construction is twice as fast as copy construction
 * - no difference in using the sgtring
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
constexpr unsigned int N = 1 << 11;  // number of (sub)strings in map/vector

const std::string empty;
std::vector<std::string> moveConstructed;
std::vector<std::string> copyConstructed;

// Not a test, juts prepare data
void BM_prepare(benchmark::State& state) 
{
    for (unsigned int i = 0; i < N; ++i) {
        std::string s;
        moveConstructed.push_back(std::move(s));
        copyConstructed.push_back(empty);
    }

    int dummy = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(++dummy);
    }
}
BENCHMARK(BM_prepare);

void BM_str_copy(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
	    std::string s = empty;
            benchmark::DoNotOptimize( s.data() );
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
            benchmark::DoNotOptimize( s.data() );
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


bool check_str_data(const std::string& s)
{
    const char* ps = s.data();
    return ps && ps[0] == 0;
}

void BM_vec_copy(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( check_str_data(copyConstructed[i]) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
void BM_vec_move(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( check_str_data(moveConstructed[i]) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_vec_copy);
BENCHMARK(BM_vec_move);
BENCHMARK(BM_vec_copy);
BENCHMARK(BM_vec_move);
BENCHMARK(BM_vec_copy);
BENCHMARK(BM_vec_move);


BENCHMARK_MAIN();
