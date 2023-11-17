
#include <lpt/spinlock.hpp>

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
constexpr const size_t numTotalLoops = 1024*1024*32;
constexpr const size_t maxNumThreads = 128;

static void BM_Pikus(benchmark::State& state) {
  static unsigned long  counterMtx{0};
  //static unsigned long* pCounterMtx = &counterMtx;
  static lpt::spinlock  spinlockCounterMtx;

  const auto nLoops{numTotalLoops/state.threads()};
  
  for (auto _ : state) {
    for (size_t n = 0; n < nLoops; ++n) {
      std::lock_guard<lpt::spinlock> lock(spinlockCounterMtx);
      benchmark::DoNotOptimize( ++counterMtx );
    }
  }
  state.counters["Rate"] = benchmark::Counter(numTotalLoops, benchmark::Counter::kAvgThreadsRate);
  state.SetComplexityN(state.threads());
}
BENCHMARK(BM_Pikus)
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


BENCHMARK_MAIN();

