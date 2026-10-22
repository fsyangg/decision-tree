#include "flat_tree.h"

#include <gtest/gtest.h>

TEST(Unit, Sanity) {
  EXPECT_EQ(1 + 2, 3);
}

TEST(UnitFlatTree, SingleClass) {
  std::vector<std::vector<int>> rows{{1}, {2}, {3}, {4}, {5}, {6}};
  std::vector<int> labels{3, 3, 3, 3, 3, 3};
  auto tree{tree::FlatTree(rows, labels)};
  tree.build_tree();
  EXPECT_EQ(tree.predict({0}), 3);
  EXPECT_EQ(tree.predict({10}), 3);
}

const std::vector<std::vector<int>> noise_and_signal{
  {0, 4},
  {0, 2},
  {1, 3},
  {1, 1},
  {0, 5},
  {1, 0}
};

TEST(UnitFlatTree, SplitsOnBestFeature) {
  std::vector<int> labels{1, 0, 1, 0, 1, 0};
  auto tree{tree::FlatTree(noise_and_signal, labels)};
  tree.build_tree();
  for (size_t i = 0; i < labels.size(); i++) {
    EXPECT_EQ(tree.predict(noise_and_signal[i]), labels[i]);
  }
  EXPECT_EQ(tree.predict({1, 2}), 0);
  EXPECT_EQ(tree.predict({0, 3}), 1);
}

TEST(UnitFlatTree, LargeLabels) {
  std::vector<int> labels{99, 8, 99, 8, 99, 8};
  auto tree{tree::FlatTree(noise_and_signal, labels)};
  tree.build_tree();
  for (size_t i = 0; i < labels.size(); i++) {
    EXPECT_EQ(tree.predict(noise_and_signal[i]), labels[i]);
  }
  EXPECT_EQ(tree.predict({1, 2}), 8);
  EXPECT_EQ(tree.predict({0, 3}), 99);
}
