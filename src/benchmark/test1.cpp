#include <benchmark/benchmark.h>

static void BM_StringCreation(benchmark::State& state) {
  for (auto _ : state)
    std::string empty_string;
}
// Register the function as a benchmark
BENCHMARK(BM_StringCreation);

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state) {
    std::string copy(x);
    benchmark::DoNotOptimize(copy);
  }
}
BENCHMARK(BM_StringCopy)
    ->ThreadRange(1, 32)
    //->ComputeStatistics("max", [](const std::vector<double>& v) -> double {
    //      return *(std::max_element(std::begin(v), std::end(v)));
    //  })
    ;

BENCHMARK_MAIN();

