#include "csv.h"
#include "tree.h"

#include <gtest/gtest.h>
#include <filesystem>

constexpr bool DEBUG{true};

TEST(Sanity, Addition) {
  EXPECT_EQ(2 + 2, 4);
}

using Data = std::pair<std::vector<std::vector<int>>, std::vector<int>>;
class TennisTest : public ::testing::Test {
protected:
  void SetUp() override {
    const auto path = std::filesystem::path{"datasets"} / "tennis.csv";
    io::CSVReader<5> in(path.string());
    int outlook = 0;
    int temperature = 0;
    int humidity = 0;
    int wind = 0;
    int play = 0;

    std::vector<std::vector<int>> row_data;
    std::vector<int> target_data;
    while (in.read_row(outlook, temperature, humidity, wind, play)) {
      row_data.push_back({outlook, temperature, humidity, wind});
      target_data.push_back(play);
    }
    data = {std::move(row_data), std::move(target_data)};
  }
  Data data;
};

TEST_F(TennisTest, LoadsTennis) {
  EXPECT_EQ(data.first.size(), 14);
  EXPECT_EQ(data.second.size(), 14);
}

TEST_F(TennisTest, TreeNotNull) {
  using namespace DTree;
  auto tree = NodeDTree(1, 1, 1);
  tree.Fit(data);
  if constexpr (DEBUG) {
    tree.Display();
  }
  EXPECT_NE(tree.Root(), nullptr);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
