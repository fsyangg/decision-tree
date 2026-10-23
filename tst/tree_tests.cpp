#include "csv.h"
#include "flat_tree.h"
#include "loaders.h"

#include <gtest/gtest.h>
#include <filesystem>

TEST(Sanity, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

using Data = std::pair<std::vector<std::vector<tree::FlatTree::sample_t>>, std::vector<tree::FlatTree::label_t>>;
class DatasetTest : public ::testing::Test
{
protected:
  double Accuracy(tree::FlatTree& tree)
  {
    double cnt{0};
    for (size_t i = 0; i < data.first.size(); i++) {
      if (tree.predict(data.first[i]) == data.second[i]) cnt++;
    }
    return cnt / data.first.size();
  }
  Data data;
};

class TennisTest : public DatasetTest
{
protected:
  void SetUp() override
  {
    const auto path = std::filesystem::path{TREE_RACE_SOURCE_DIR} / "datasets" / "tennis.csv";
    io::CSVReader<5> in(path.string());
    std::array<tree::FlatTree::sample_t, 5> a;
    std::vector<std::vector<tree::FlatTree::sample_t>> row_data;
    std::vector<tree::FlatTree::label_t> target_data;
    while (in.read_row(a[0], a[1], a[2], a[3], a[4])) {
      row_data.push_back({a[0], a[1], a[2], a[3]});
      target_data.push_back(a[4]);
    }
    data = {std::move(row_data), std::move(target_data)};
  }
};

class CarTest : public DatasetTest
{
protected:
  void SetUp() override
  {
    data = tree::load_car<tree::FlatTree::sample_t, tree::FlatTree::label_t>();
  }
};

TEST_F(TennisTest, LoadsTennis) {
  EXPECT_EQ(data.first.size(), 14);
  EXPECT_EQ(data.second.size(), 14);
}

TEST_F(CarTest, LoadsCar) {
  EXPECT_EQ(data.first.size(), 1728);
  EXPECT_EQ(data.second.size(), 1728);
}

TEST_F(TennisTest, FlatTree) {
  auto tree = tree::FlatTree(data.first, data.second);
  tree.build_tree();
  auto acc{Accuracy(tree)};
  std::cout << "accuracy: " << acc << std::endl;
  EXPECT_GT(acc, 0.5);
}

TEST_F(CarTest, FlatTree) {
  auto tree = tree::FlatTree(data.first, data.second);
  tree.build_tree();
  auto acc{Accuracy(tree)};
  std::cout << "accuracy: " << acc << std::endl;
  EXPECT_GT(acc, 0.95);
}
