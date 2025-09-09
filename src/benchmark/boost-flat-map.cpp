/*
 * boost unordered flat map
 */

#include <boost/unordered/concurrent_flat_map.hpp>

#include <benchmark/benchmark.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

//-----------------------------------------------------------------------------

constexpr const size_t numTotalLoops = 1024*1024*32;
constexpr const size_t maxNumThreads = 128;

using rlock_t = std::shared_lock<std::shared_mutex>;
using wlock_t = std::unique_lock<std::shared_mutex>;

boost::concurrent_flat_map<uint64_t, uint64_t> g_cfmMap;

std::shared_mutex g_mutexMap;
std::unordered_map<uint64_t, uint64_t> g_stdMap;


static void BM_unordered_map_setup(benchmark::State& state) {

  const auto nLoops{numTotalLoops/state.threads()};
  const auto nThreadIdx(state.thread_index());
  const auto lbound(nThreadIdx*nLoops); //[
  const auto ubound((nThreadIdx+1)*nLoops); //)
  
  if (state.thread_index() == 0) {
  }
  
  for (auto _ : state) {
    for (size_t n = lbound; n < ubound; ++n) {
      wlock_t lock(g_mutexMap);
      benchmark::DoNotOptimize( g_stdMap.emplace(n, n) );
    }
  }

  if (state.thread_index() == 0) {
  }
  
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_unordered_map_setup)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;


static void BM_boost_map_setup(benchmark::State& state) {

  const auto nLoops{numTotalLoops/state.threads()};
  const auto nThreadIdx(state.thread_index());
  const auto lbound(nThreadIdx*nLoops); //[
  const auto ubound((nThreadIdx+1)*nLoops); //)
  
  if (state.thread_index() == 0) {
  }
  
  for (auto _ : state) {
    for (size_t n = lbound; n < ubound; ++n) {
      benchmark::DoNotOptimize( g_cfmMap.emplace(n, n) );
    }
  }

  if (state.thread_index() == 0) {
  }
  
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_boost_map_setup)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;


static void BM_unordered_map_read(benchmark::State& state) {

  const auto nLoops{numTotalLoops/state.threads()};
  const auto nThreadIdx(state.thread_index());
  const auto lbound(nThreadIdx*nLoops); //[
  const auto ubound((nThreadIdx+1)*nLoops); //)
  
  if (state.thread_index() == 0) {
    for (size_t n = 0; n < numTotalLoops; ++n) {
      g_stdMap.emplace(n, n);
    }
  }
  
  uint64_t val;
  for (auto _ : state) {
    for (size_t n = lbound; n < ubound; ++n) {
      rlock_t lock(g_mutexMap);
      benchmark::DoNotOptimize( val = g_stdMap[n] );
    }
  }

  if (state.thread_index() == 0) {
    g_stdMap.clear();
  }
  
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_unordered_map_read)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;


static void BM_boost_map_read(benchmark::State& state) {

  const auto nLoops{numTotalLoops/state.threads()};
  const auto nThreadIdx(state.thread_index());
  const auto lbound(nThreadIdx*nLoops); //[
  const auto ubound((nThreadIdx+1)*nLoops); //)
  
  if (state.thread_index() == 0) {
    for (size_t n = 0; n < numTotalLoops; ++n) {
      g_cfmMap.emplace(n, n);
    }
  }
  
  uint64_t val;
  for (auto _ : state) {
    for (size_t n = lbound; n < ubound; ++n) {
      benchmark::DoNotOptimize( g_cfmMap.cvisit(n/*key*/, [&val](const auto& pair){val=pair.second;}) );
    }
  }

  if (state.thread_index() == 0) {
    g_cfmMap.clear();
  }
  
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_boost_map_read)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;


BENCHMARK_MAIN();

