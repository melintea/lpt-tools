#include <benchmark/benchmark.h>

#include <atomic>
#include <iostream>
#include <mutex>

#include <pthread.h>

//-----------------------------------------------------------------------------

class PtreadsSpinLockWrap
{
public:

    PtreadsSpinLockWrap()  { pthread_spin_init(&_lock, PTHREAD_PROCESS_PRIVATE); }
    ~PtreadsSpinLockWrap() { pthread_spin_destroy(&_lock); }

    PtreadsSpinLockWrap(const PtreadsSpinLockWrap&)            = delete;
    PtreadsSpinLockWrap& operator=(const PtreadsSpinLockWrap&) = delete;

    PtreadsSpinLockWrap(PtreadsSpinLockWrap&&)                 = delete;
    PtreadsSpinLockWrap& operator=(PtreadsSpinLockWrap&&)      = delete;

    void lock()   { pthread_spin_lock(&_lock);   }
    void unlock() { pthread_spin_unlock(&_lock); }

private:

    pthread_spinlock_t _lock;
};

//-----------------------------------------------------------------------------
constexpr const int numTotalLoops = 1024*1024*32;
constexpr const int maxNumThreads = 128;

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

static void BM_PtSpin(benchmark::State& state) {
  static unsigned long        counterMtx{0};
  //static unsigned long*       pCounterMtx = &counterMtx;
  static PtreadsSpinLockWrap  mutexCounterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::lock_guard<PtreadsSpinLockWrap> lock(mutexCounterMtx);
      benchmark::DoNotOptimize( ++counterMtx );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_PtSpin)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    ;

static void BM_LockFree(benchmark::State& state) {
  static std::atomic<unsigned long>  counter{0};
  //static std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      unsigned long x = counter.load(std::memory_order_relaxed);
      while ( ! counter.compare_exchange_strong(x, x+1, std::memory_order_relaxed, std::memory_order_relaxed) );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_LockFree)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    ;

static void BM_WaitFree(benchmark::State& state) {
  static std::atomic<unsigned long>  counter{0};
  //static std::atomic<unsigned long>* pCounter = &counter;
  
  const auto nLoops{numTotalLoops/state.threads()};
  //std::cout << nLoops << "\n";
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      benchmark::DoNotOptimize( counter.fetch_add(1, std::memory_order_relaxed) );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_WaitFree)
    ->ThreadRange(1, maxNumThreads)
    ->UseRealTime()
    ->Complexity(benchmark::oAuto)
    ;

BENCHMARK_MAIN();

