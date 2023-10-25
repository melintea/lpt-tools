#include <benchmark/benchmark.h>

#include <atomic>
#include <mutex>

constexpr const int numLoops=1'000'000'000;

static void BM_Mutex(benchmark::State& state) {
  unsigned long  counter{0};
  unsigned long* pCounter = &counter;
  std::mutex     counterMtx;
  
  for (auto _ : state) {
    std::lock_guard<std::mutex> lock(counterMtx);
    benchmark::DoNotOptimize( ++(*pCounter) );
    //if (*pCounter == numLoops) {
    //  break;
    //}
  }
}
BENCHMARK(BM_Mutex)
    ->ThreadRange(1, 32)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;

static void BM_LockFree(benchmark::State& state) {
  std::atomic<unsigned long>  counter{0};
  std::atomic<unsigned long>* pCounter = &counter;
  
  for (auto _ : state) {
    unsigned long x = pCounter->load(std::memory_order_relaxed);
    while ( ! pCounter->compare_exchange_strong(x, x+1, std::memory_order_relaxed, std::memory_order_relaxed) );
  }
}
BENCHMARK(BM_LockFree)
    ->ThreadRange(1, 32)
    ;

static void BM_WaitFree(benchmark::State& state) {
  std::atomic<unsigned long>  counter{0};
  std::atomic<unsigned long>* pCounter = &counter;
  
  for (auto _ : state) {
    benchmark::DoNotOptimize( pCounter->fetch_add(1, std::memory_order_relaxed) );
  }
}
BENCHMARK(BM_WaitFree)
    ->ThreadRange(1, 32)
    ;

BENCHMARK_MAIN();

