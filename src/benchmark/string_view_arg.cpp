/*
 * string_view vs const& as arg
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
std::vector<std::string> vecs;
std::vector<const char*> vecp;

int xdummy = 0;

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
        vecp.push_back(ps);
        vecs.push_back(ps);
    }

    int dummy = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(++dummy);
    }
}
BENCHMARK(BM_prepare);

int dummy_call(const std::string& s)
{
    if ( ! s.empty()) {++xdummy;}
    return xdummy;
}

int dummy_call(std::string_view s)
{
    if ( ! s.empty()) {++xdummy;}
    return xdummy;
}

template <typename FIND_T>
void BM_arg(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( dummy_call(vecs[i]) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
template <>
void BM_arg<std::string_view>(benchmark::State& state) {
    
    for (auto _ : state) {
        for (int i = 0; i < static_cast<int>(N); ++i) {
            //std::cout << &s[i] << '\n';
            benchmark::DoNotOptimize( dummy_call(std::string_view(vecs[i])) );
        }
    }
    state.SetItemsProcessed(N*state.iterations());
}
BENCHMARK(BM_arg<std::string>);
BENCHMARK(BM_arg<std::string_view>);
BENCHMARK(BM_arg<std::string>);
BENCHMARK(BM_arg<std::string_view>);
BENCHMARK(BM_arg<std::string>);
BENCHMARK(BM_arg<std::string_view>);
BENCHMARK(BM_arg<std::string>);
BENCHMARK(BM_arg<std::string_view>);


BENCHMARK_MAIN();
