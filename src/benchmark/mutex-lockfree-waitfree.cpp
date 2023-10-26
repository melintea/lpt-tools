#include <benchmark/benchmark.h>

#include <atomic>
#include <iostream>
#include <mutex>

constexpr const int numTotalLoops = 1024*1024*32;
constexpr const int maxNumThreads = 128;

static void BM_Mutex(benchmark::State& state) {
  unsigned long  counter{0};
  unsigned long* pCounter = &counter;
  std::mutex     counterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::lock_guard<std::mutex> lock(counterMtx);
      benchmark::DoNotOptimize( ++(*pCounter) );
    }
  }
}
BENCHMARK(BM_Mutex)
    ->ThreadRange(1, maxNumThreads)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;

static void BM_LockFree(benchmark::State& state) {
  std::atomic<unsigned long>  counter{0};
  std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      unsigned long x = pCounter->load(std::memory_order_relaxed);
      while ( ! pCounter->compare_exchange_strong(x, x+1, std::memory_order_relaxed, std::memory_order_relaxed) );
    }
  }
}
BENCHMARK(BM_LockFree)
    ->ThreadRange(1, maxNumThreads)
    ;

static void BM_WaitFree(benchmark::State& state) {
  std::atomic<unsigned long>  counter{0};
  std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  //std::cout << nLoops << "\n";
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      benchmark::DoNotOptimize( pCounter->fetch_add(1, std::memory_order_relaxed) );
    }
  }
}
BENCHMARK(BM_WaitFree)
    ->ThreadRange(1, maxNumThreads)
    ;

BENCHMARK_MAIN();

