#include "flat_tree.h"
#include "node_tree.h"
#include "loaders.h"

#include <benchmark/benchmark.h>

namespace {

void BM_node_tree__fit_car(benchmark::State &state) {
  auto data{tree::load_car()};
  auto tree{tree::NodeTree(5, 1, 100)};

  for (auto _ : state) {
    tree.Fit(data);
    benchmark::DoNotOptimize(tree.Root());
  }
}

void BM_flat_tree__fit_car(benchmark::State &state) {
  auto data{tree::load_car<tree::FlatTree::sample_t, tree::FlatTree::label_t>()};
  tree::FlatTree tree{data.first, data.second};

  for (auto _ : state) {
    tree.build_tree();
    benchmark::DoNotOptimize(tree);
  }
}

void BM_node_tree__predict_car(benchmark::State &state) {
  auto data{tree::load_car()};
  auto tree{tree::NodeTree(5, 1, 100)};
  tree.Fit(data);

  for (auto _ : state) {
    for (auto &row : data.first) {
      benchmark::DoNotOptimize(tree.Predict(row));
    }
  }
}

void BM_flat_tree__predict_car(benchmark::State &state) {
  auto data{tree::load_car<tree::FlatTree::sample_t, tree::FlatTree::label_t>()};
  tree::FlatTree tree{data.first, data.second};
  tree.build_tree();

  for (auto _ : state) {
    for (auto &row : data.first) {
      benchmark::DoNotOptimize(tree.predict(row));
    }
  }
}

} // namespace

BENCHMARK(BM_node_tree__fit_car);
BENCHMARK(BM_flat_tree__fit_car)->Repetitions(5)->ReportAggregatesOnly();
BENCHMARK(BM_node_tree__predict_car);
BENCHMARK(BM_flat_tree__predict_car)->Repetitions(5)->ReportAggregatesOnly();
BENCHMARK_MAIN();
