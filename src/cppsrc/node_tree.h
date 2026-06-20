#pragma once

#include "tree.h"

#include <numeric>
#include <algorithm>
#include <queue>
#include <iostream>

namespace DTree
{
struct Node
{
  bool is_leaf;
  int feature_index;
  float threshold;
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;
  int predicted_class;
};

inline std::ostream& operator<<(std::ostream& os, const Node& dn) {
  os << "Node{is_leaf=" << dn.is_leaf;
  os << ", feature_index=" << dn.feature_index;
  os << ", threshold=" << dn.threshold;
  os << ", has_left=" << (dn.left==nullptr?"false":"true");
  os << ", has_right=" << (dn.right==nullptr?"false":"true");
  if (dn.is_leaf) os << ", predicted_class=" << dn.predicted_class;
  os << "}";
  return os;
}

class NodeTree : public DecisionTree
{
public:
  NodeTree(int min_samples_split, int min_samples_leaf, int max_depth)
      : min_samples_(min_samples_split),
        min_samples_leaf_(min_samples_leaf),
        max_depth_(max_depth) {}

  void Fit(const Data& data) override
  {
    X_type X {data.first};
    Y_type Y {data.second}; 
    root_ = BuildTree(X, Y, 0);
  }

  int Predict(const std::vector<int>& f_row) override
  {
    Node* c = root_.get();
    while (!c->is_leaf) {
      int f{c->feature_index};
      float t{c->threshold};
      if ((float)f_row[f] >= t) {
        c = c->right.get();
      } else {
        c = c->left.get();
      }
    }
    return c->predicted_class;
  }

  Node* Root()
  {
    return root_.get();
  }

  void Display()
  {
    std::queue<Node*> q;
    q.push(root_.get());
    int cnt {0};
    while (!q.empty()) {
      std::cout << "level " << cnt++ << '\n';
      int sz = q.size();
      for (int i = 0; i < sz; i++) {
        Node* top = q.front(); q.pop();
        std::cout << *top << '\n';
        if (top->left != nullptr) q.push(top->left.get());
        if (top->right != nullptr) q.push(top->right.get());
      }
    }
    std::cout << "done\n";
  }

  std::pair<int, float> PublicFindBestThreshold(const X_type& X, const Y_type& Y, int K)
  {
    return FindBestThreshold(X, Y, K);
  }

// private:
  std::unique_ptr<Node> BuildTree(const X_type& X, const Y_type& Y, int depth)
  {
    int K = NumCategories(Y);
    if (StoppingCondition(X, Y, depth)) {
      std::vector<int> cnt(Y);
      int mx{0}, mxcnt{0};
      for (auto x : Y) {
        if (++cnt[x] > mxcnt) {
          mxcnt = cnt[x];
          mx = x;
        }
      }
      return std::make_unique<Node>(Node{true, -1, 0.0, nullptr, nullptr, mx});
    }

    auto [best_feature_idx, best_threshold] = FindBestThreshold(X, Y, K);
    X_type X_left, X_right;
    Y_type Y_left, Y_right;

    for (int i = 0; i < X.size(); i++) {
      if (X[i][best_feature_idx] < best_threshold) {
        X_left.push_back(X[i]);
        Y_left.push_back(Y[i]);
      } else {
        X_right.push_back(X[i]);
        Y_right.push_back(Y[i]);
      }
    }

    auto node {std::make_unique<Node>(Node{false, best_feature_idx, best_threshold})};
    node->left = BuildTree(X_left, Y_left, depth + 1);
    node->right = BuildTree(X_right, Y_right, depth + 1);

    return node;
  }

  int NumCategories(const Y_type& Y)
  {
    std::vector<int> a(Y.size(), 0);
    int cnt {0};
    for (auto& elt : Y) {
      cnt += (a[elt]++ == 0);
    }
    return cnt;
  }

  std::pair<int, float> FindBestThreshold(const X_type& X, const Y_type& Y, int K)
  {
    const auto fcnt{X[0].size()};
    const auto n{X.size()};

    int best_feature{-1};
    float best_threshold{-1};
    float best_score{2.0};

    for (int f_idx = 0; f_idx < fcnt; f_idx++) {
      auto indices {GetIndexArray(X, f_idx)};

      // assume categories are numbered 0...n-1
      // then there are at most n-1 categories
      std::vector<int> right_counts(K, 0);
      std::vector<int> left_counts(K, 0);
      for (auto e : Y) right_counts[e]++;
      for (int i = 0; i < indices.size(); i++) {
        // something is wrong here
        int sorted_idx{indices[i]};
        int x{X[sorted_idx][f_idx]};
        int y{Y[sorted_idx]};
        right_counts[y]--;
        left_counts[y]++;

        float score{(Gini(right_counts, n-i-1) + Gini(left_counts, i+1)) / 2};
        if (score < best_score) {
          best_score = score;
          best_feature = f_idx;
          // TODO: this can go out of bounds, when i == indices.size() - 1
          best_threshold = ((float)X[indices[i+1]][f_idx] + x) / 2;
        }
      }
    }
    return {best_feature, best_threshold};
  }

  std::vector<int> GetIndexArray(const X_type& X, int feature_index)
  {
    std::vector<int> idx(X.size());
    std::iota(idx.begin(), idx.end(), 0);

    std::sort(idx.begin(), idx.end(),
        [&](int a, int b){ return X[a][feature_index] < X[b][feature_index]; });

    return idx;
  }

  float Gini(const std::vector<int>& counts, int n)
  {
    if (n == 0) return 0.0;
    float sum_sq {0.0};
    for (const auto e : counts) {
      float ratio {e / (float)n};
      sum_sq += ratio * ratio;
    }
    return 1.0 - sum_sq;
  }

  bool StoppingCondition(const X_type& X, const Y_type& Y, int depth)
  {
    const int n = Y.size();

    if (n < min_samples_) return true;
    if (n < 2 * min_samples_leaf_) return true;
    if (depth > max_depth_) return true;
    if (std::adjacent_find(Y.begin(), Y.end(), std::not_equal_to<>{}) == Y.end()) return true;
    // TODO: if no gain (or negative gain) by any split, we're done

    return false;
  }

  std::unique_ptr<Node> root_;
  int min_samples_;
  int min_samples_leaf_;
  int max_depth_;
};
} // namespace DTree
