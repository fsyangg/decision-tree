#include "csv.h"
#include "node_tree.h"

#include <benchmark/benchmark.h>

namespace {

void BM_node_tree__fit_car(benchmark::State &state) {
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
  DTree::Data data{std::move(row_data), std::move(target_data)};
  auto tree{DTree::NodeTree(2, 1, 100)};

  for (auto _ : state) {
    tree.Fit(data);
    benchmark::DoNotOptimize(tree.Root());
  }
}

void BM_node_tree__fit_tennis(benchmark::State &state) {
  const auto path =
      std::filesystem::path{TREE_RACE_SOURCE_DIR} / "datasets" / "tennis.csv";
  io::CSVReader<5> in(path.string());
  std::array<int, 5> a;
  std::vector<std::vector<int>> row_data;
  std::vector<int> target_data;
  while (in.read_row(a[0], a[1], a[2], a[3], a[4])) {
    row_data.push_back({a[0], a[1], a[2], a[3]});
    target_data.push_back(a[4]);
  }
  DTree::Data data = {std::move(row_data), std::move(target_data)};
  auto tree{DTree::NodeTree(2, 1, 100)};

  for (auto _ : state) {
    tree.Fit(data);
    benchmark::DoNotOptimize(tree.Root());
  }
}

void BM_node_tree__fit_mushroom(benchmark::State &state) {
  const auto path =
      std::filesystem::path{TREE_RACE_SOURCE_DIR} / "datasets" / "mushroom.csv";
  io::CSVReader<23> in(path.string());
  std::array<int, 23> a;
  std::vector<std::vector<int>> row_data;
  std::vector<int> target_data;
  while (in.read_row(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9],
                     a[10], a[11], a[12], a[13], a[14], a[15], a[16], a[17],
                     a[18], a[19], a[20], a[21], a[22])) {
    row_data.push_back({a[1],  a[2],  a[3],  a[4],  a[5],  a[6],  a[7],  a[8],
                        a[9],  a[10], a[11], a[12], a[13], a[14], a[15], a[16],
                        a[17], a[18], a[19], a[20], a[21], a[22]});
    target_data.push_back(a[0]);
  }
  DTree::Data data = {std::move(row_data), std::move(target_data)};
  auto tree{DTree::NodeTree(2, 1, 100)};

  for (auto _ : state) {
    tree.Fit(data);
    benchmark::DoNotOptimize(tree.Root());
  }
}

} // namespace

BENCHMARK(BM_node_tree__fit_car);
BENCHMARK(BM_node_tree__fit_tennis);
BENCHMARK(BM_node_tree__fit_mushroom);
BENCHMARK_MAIN();
