#include "csv.h"
#include "node_tree.h"

#include <gtest/gtest.h>
#include <filesystem>

constexpr bool DEBUG{false};

TEST(Sanity, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

using Data = std::pair<std::vector<std::vector<int>>, std::vector<int>>;
class DatasetTest : public ::testing::Test
{
protected:
  double Accuracy(const std::vector<int>& predictions)
  {
    double cnt{0};
    double n{static_cast<double>(predictions.size())};
    for (int i = 0; i < predictions.size(); i++) {
      if (predictions[i] == data.second[i]) cnt++;
    }
    if constexpr (DEBUG) {
      std::cout << predictions.size() << " samples, " << cnt << " correct\n";
    }
    return cnt / n;
  }
  tree::Data data;
};

class TennisTest : public DatasetTest
{
protected:
  void SetUp() override
  {
    const auto path = std::filesystem::path{"/Users/fsyang/src/dtree/datasets/tennis.csv"};
    io::CSVReader<5> in(path.string());
    std::array<int, 5> a;
    std::vector<std::vector<int>> row_data;
    std::vector<int> target_data;
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
    const auto path = std::filesystem::path{"/Users/fsyang/src/dtree/datasets/car_eval.csv"};
    io::CSVReader<7> in(path.string());
    std::array<int, 7> a;
    std::vector<std::vector<int>> row_data;
    std::vector<int> target_data;
    while (in.read_row(a[0], a[1], a[2], a[3], a[4], a[5], a[6])) {
      row_data.push_back({a[0], a[1], a[2], a[3], a[4], a[5]});
      target_data.push_back(a[6]);
    }
    data = {std::move(row_data), std::move(target_data)};
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

TEST_F(TennisTest, TreeNotNull) {
  auto tree = tree::NodeTree(1, 1, 1);
  tree.Fit(data);
  if constexpr (DEBUG) {
    tree.Display();
  }
  EXPECT_NE(tree.Root(), nullptr);
}

TEST_F(CarTest, NodeTree) {
  auto tree = tree::NodeTree(4, 4, 16);
  tree.Fit(data);
  if constexpr (DEBUG) {
    tree.Display();
  }
  EXPECT_NE(tree.Root(), nullptr);
  std::vector<int> predictions;
  for (const auto& row : data.first) {
    predictions.push_back(tree.Predict(row));
  }
  auto acc{Accuracy(predictions)};
  std::cout << "accuracy: " << acc << std::endl;
  EXPECT_GT(Accuracy(predictions), 0.5);
}

TEST_F(TennisTest, SimplePrediction) {
  auto tree = tree::NodeTree(1, 1, 10);
  tree.Fit(data);
  std::vector<int> predictions;
  for (const auto& row : data.first) {
    predictions.push_back(tree.Predict(row));
  }
  auto acc{Accuracy(predictions)};
  std::cout << "accuracy: " << acc << std::endl;
  EXPECT_GT(Accuracy(predictions), 0.5);
}
