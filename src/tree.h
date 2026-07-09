#pragma once

#include <vector>

namespace DTree
{
using X_type = std::vector<std::vector<int>>;
using Y_type = std::vector<int>;
using Data = std::pair<X_type, Y_type>;

class DecisionTree
{
public:
  virtual void Fit(const Data& data) = 0;
  virtual int Predict(const std::vector<int>& feature_row) = 0;
  virtual ~DecisionTree() {}
};
} // namespace DTree
