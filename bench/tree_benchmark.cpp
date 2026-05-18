#include <benchmark/benchmark.h>

namespace {

void benchmark_train(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(4 + 4);
  }
}

} // namespace

BENCHMARK(benchmark_train);
BENCHMARK_MAIN();
