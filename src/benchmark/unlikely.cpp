/*
 * un/likely & branchless tests
 * Test: ./run_bm.sh unlikely.cpp
 */
 
#include <iostream>
#include <stdlib.h>
#include <string.h>

#define MCA_START __asm volatile("# LLVM-MCA-BEGIN");
#define MCA_END   __asm volatile("# LLVM-MCA-END");

#include <benchmark/benchmark.h>


void BM_branched1(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3; // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i];
            } else {
                a2 += p2[i];
            }
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

void BM_branchless1(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3;  // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            unsigned long s1[2] = {     0, p1[i] };
            unsigned long s2[2] = { p2[i],     0 };
            a1 += s1[b1[i]];
            a2 += s2[b1[i]];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

// randomish reference 
void BM_branched2(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3;  // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i] - p2[i];
            } else {
                a2 += p1[i] * p2[i];
            }
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

// randomish un/likely correctly tagged
void BM_branched2_unlikely_ok(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3;  // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if ( ! b1[i]) [[unlikely]] {
                a1 += p1[i] - p2[i];
            } else [[likely]] {
                a2 += p1[i] * p2[i];
            }
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

// randomish un/likely INcorrectly tagged
void BM_branched2_unlikely_KO(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3;  // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if ( ! b1[i]) [[likely]] {
                a1 += p1[i] - p2[i];
            } else [[unlikely]] {
                a2 += p1[i] * p2[i];
            }
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

// CPU branch predictor in full on unrandomish - this is the limit to aim for
void BM_branched2_predicted(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() > 0;  // bools in disguise, overhelmlingly true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (b1[i]) {
                a1 += p1[i] - p2[i];
            } else {
                a2 += p1[i] * p2[i];
            }
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

// randomish branchless (@see Fedor Pikus)
void BM_branchless2(benchmark::State& state) {
    srand(1);
    const unsigned int N = state.range(0);
    std::vector<unsigned long> v1(N), v2(N);
    std::vector<int> c1(N);
    for (size_t i = 0; i < N; ++i) {
        v1[i] = rand();
        v2[i] = rand();
        c1[i] = rand() % 3;  // bools in disguise, 2/3 true
    }
    unsigned long* p1 = v1.data();
    unsigned long* p2 = v2.data();
    int* b1 = c1.data();
    for (auto _ : state) {
        unsigned long a1 = 0, a2 = 0;
        for (size_t i = 0; i < N; ++i) {
            unsigned long s1[2] = {             0, p1[i] - p2[i] };
            unsigned long s2[2] = { p1[i] * p2[i],             0 };
            a1 += s1[b1[i]];
            a2 += s2[b1[i]];
        }
        benchmark::DoNotOptimize(a1);
        benchmark::DoNotOptimize(a2);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(N*state.iterations());
    //state.SetBytesProcessed(N*sizeof(unsigned long)*state.iterations());
}

#define ARGS \
    ->Arg(1<<22)

BENCHMARK(BM_branched1) ARGS;
BENCHMARK(BM_branchless1) ARGS;
BENCHMARK(BM_branched2) ARGS;
BENCHMARK(BM_branched2_predicted) ARGS;
BENCHMARK(BM_branchless2) ARGS;
BENCHMARK(BM_branched2_unlikely_ok) ARGS;
BENCHMARK(BM_branched2_unlikely_KO) ARGS;

BENCHMARK_MAIN();

