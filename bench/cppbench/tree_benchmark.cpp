#include "csv.h"
#include "flat_tree.h"
#include "node_tree.h"

#include <benchmark/benchmark.h>
#include <filesystem>

namespace {

DTree::Data load_car() {
  const auto path =
      std::filesystem::path{TREE_RACE_SOURCE_DIR} / "datasets" / "car_eval.csv";
  io::CSVReader<7> in(path.string());
  std::array<int, 7> a;
  std::vector<std::vector<int>> row_data;
  std::vector<int> target_data;
  while (in.read_row(a[0], a[1], a[2], a[3], a[4], a[5], a[6])) {
    row_data.push_back({a[0], a[1], a[2], a[3], a[4], a[5]});
    target_data.push_back(a[6]);
  }
  return {std::move(row_data), std::move(target_data)};
}

void BM_node_tree__fit_car(benchmark::State &state) {
  auto data{load_car()};
  auto tree{DTree::NodeTree(5, 1, 100)};

  for (auto _ : state) {
    tree.Fit(data);
    benchmark::DoNotOptimize(tree.Root());
  }
}

void BM_flat_tree__fit_car(benchmark::State &state) {
  auto data{load_car()};
  tree::FlatTree tree{data.first, data.second};

  for (auto _ : state) {
    tree.build_tree();
    benchmark::DoNotOptimize(tree);
  }
}

void BM_node_tree__predict_car(benchmark::State &state) {
  auto data{load_car()};
  auto tree{DTree::NodeTree(5, 1, 100)};
  tree.Fit(data);

  for (auto _ : state) {
    for (auto &row : data.first) {
      benchmark::DoNotOptimize(tree.Predict(row));
    }
  }
}

void BM_flat_tree__predict_car(benchmark::State &state) {
  auto data{load_car()};
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
BENCHMARK(BM_flat_tree__fit_car);
BENCHMARK(BM_node_tree__predict_car);
BENCHMARK(BM_flat_tree__predict_car);
BENCHMARK_MAIN();
