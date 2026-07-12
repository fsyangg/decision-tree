#include "node_tree.h"

#include <gtest/gtest.h>

TEST(Unit, Sanity) {
  EXPECT_EQ(1 + 2, 3);
}

TEST(UnitNodeTree, GetIndexArray) {
  auto tree{DTree::NodeTree(1, 1, 1)};
  std::vector<std::vector<int>> rows{
    {1, 2, 3},
    {2, 3, 1},
    {3, 1, 2}
  };
  std::vector<std::vector<int>> expected{
    {0, 1, 2},
    {2, 0, 1},
    {1, 2, 0}
  };
  EXPECT_EQ(tree.GetIndexArray(rows, 0), expected[0]);
  EXPECT_EQ(tree.GetIndexArray(rows, 1), expected[1]);
  EXPECT_EQ(tree.GetIndexArray(rows, 2), expected[2]);
}

TEST(UnitNodeTree, Gini) {
  auto tree{DTree::NodeTree(1, 1, 1)};
  std::vector<int> cnt1{1, 1};
  EXPECT_EQ(tree.Gini(cnt1, 2), 0.5);
  std::vector<int> cnt2{5, 0, 0, 0, 0};
  EXPECT_EQ(tree.Gini(cnt2, 5), 0.0);
}

TEST(UnitNodeTree, FindBestSplit) {
  std::vector<std::vector<int>> feature_rows{
    {4, 0, 0, 0},
    {2, 0, 0, 0},
    {3, 0, 0, 0},
    {1, 0, 0, 0},
    {5, 0, 0, 0},
    {0, 0, 0, 0}
  };
  std::vector<int> labels{
    1,
    0,
    1,
    0,
    1,
    0
  };
  auto tree{DTree::NodeTree(1, 1, 1)};
  auto [idx, threshold] = tree.FindBestThreshold(feature_rows, labels, 2);
  EXPECT_EQ(idx, 0);
  EXPECT_EQ(threshold, 2.5);
}
