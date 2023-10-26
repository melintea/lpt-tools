#include <benchmark/benchmark.h>

#include <atomic>
#include <iostream>
#include <mutex>

#include <pthread.h>

constexpr const int numTotalLoops = 1024*1024*32;
constexpr const int maxNumThreads = 128;

static void BM_Mutex(benchmark::State& state) {
  static unsigned long  counterMtx{0};
  static unsigned long* pCounterMtx = &counterMtx;
  static std::mutex     mutexCounterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::lock_guard<std::mutex> lock(mutexCounterMtx);
      benchmark::DoNotOptimize( ++(*pCounterMtx) );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
}
BENCHMARK(BM_Mutex)
    ->ThreadRange(1, maxNumThreads)
    ->MeasureProcessCPUTime()->UseRealTime()
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;

static void BM_LockFree(benchmark::State& state) {
  static std::atomic<unsigned long>  counter{0};
  static std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      unsigned long x = pCounter->load(std::memory_order_relaxed);
      while ( ! pCounter->compare_exchange_strong(x, x+1, std::memory_order_relaxed, std::memory_order_relaxed) );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
}
BENCHMARK(BM_LockFree)
    ->ThreadRange(1, maxNumThreads)
    ->MeasureProcessCPUTime()->UseRealTime()
    ;

static void BM_WaitFree(benchmark::State& state) {
  static std::atomic<unsigned long>  counter{0};
  static std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  //std::cout << nLoops << "\n";
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      benchmark::DoNotOptimize( pCounter->fetch_add(1, std::memory_order_relaxed) );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
}
BENCHMARK(BM_WaitFree)
    ->ThreadRange(1, maxNumThreads)
    ->MeasureProcessCPUTime()->UseRealTime()
    ;

BENCHMARK_MAIN();

