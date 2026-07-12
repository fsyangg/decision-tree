#include "node_tree.h"
#include "csv.h"

#include <benchmark/benchmark.h>

namespace {

void benchmark_train(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(4 + 4);
  }
}

void bench_fit_car_eval(benchmark::State &state) {

}

} // namespace

BENCHMARK(benchmark_train);
BENCHMARK_MAIN();
