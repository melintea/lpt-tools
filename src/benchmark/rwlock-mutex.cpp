/*
 * std::share_mutex/rw-lock vs mutex
 */

#include <benchmark/benchmark.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <shared_mutex>

//-----------------------------------------------------------------------------
constexpr const size_t numTotalLoops = 1024*1024*32;
constexpr const size_t maxNumThreads = 128;

static void BM_Mutex(benchmark::State& state) {
  static unsigned long  counterMtx{0};
  //static unsigned long* pCounterMtx = &counterMtx;
  static std::mutex     mutexCounterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::lock_guard<std::mutex> lock(mutexCounterMtx);
      benchmark::DoNotOptimize( ++counterMtx );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_Mutex)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;

static void BM_RWLock(benchmark::State& state) {
  static unsigned long     counterMtx{0};
  //static unsigned long* pCounterMtx = &counterMtx;
  static std::shared_mutex mutexCounterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::shared_lock lock(mutexCounterMtx);
      benchmark::DoNotOptimize( ++counterMtx );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_RWLock)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;


BENCHMARK_MAIN();

