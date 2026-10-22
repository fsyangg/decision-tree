#pragma once

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <stack>
#include <vector>

namespace tree
{

class FlatTree
{
   public:
    FlatTree(const std::vector<std::vector<int>>& data, const std::vector<int>& labels) : data_{data}, labels_{labels}
    {
    }
    void build_tree();
    int predict(const std::vector<int>& row);

   private:
    std::pair<int, float> split(size_t index, size_t start, size_t end);

    template<size_t ArrSize>
    float gini_entropy(std::array<int, ArrSize> counts, size_t n);

    size_t hoare_partition(size_t start, size_t end, int feature, float threshold);

    std::reference_wrapper<const std::vector<std::vector<int>>> data_;
    std::reference_wrapper<const std::vector<int>> labels_;

    std::vector<int> left_child_;
    std::vector<int> right_child_;
    std::vector<int> feature_;
    std::vector<float> threshold_;

    std::vector<bool> is_leaf_;
    std::vector<int> samples_;

    size_t num_attr_;
    [[maybe_unused]] size_t num_categories_;
};

inline int FlatTree::predict(const std::vector<int>& sample)
{
    ZoneScoped;
    size_t node{0};
    while (!is_leaf_[node])
    {
        if (sample[feature_[node]] <= threshold_[node])
        {
            node = node * 2 + 1;
        }
        else
        {
            node = node * 2 + 2;
        }
    }
    assert(node > 0 && node < feature_.size());
    return feature_[node];
}

inline void FlatTree::build_tree()
{
    ZoneScoped;
    auto& data{data_.get()};
    auto& labels{labels_.get()};
    num_attr_ = data[0].size();

    samples_.resize(labels.size());
    for (size_t i{0}; i < labels.size(); ++i)
    {
        samples_[i] = i;
    }

    std::stack<std::tuple<size_t, size_t, size_t>> stack;
    stack.push({0, 0, labels.size()});

    while (!stack.empty())
    {
        auto [index, start, end] = stack.top();
        stack.pop();

        if (index >= is_leaf_.size())
        {
            is_leaf_.resize(index + 1);
            feature_.resize(index + 1);
            threshold_.resize(index + 1);
        }

        auto [best_feat, best_threshold] = split(index, start, end);
        feature_[index] = best_feat;
        threshold_[index] = best_threshold;

        if (!is_leaf_[index])
        {
            auto partition_pos{hoare_partition(start, end, best_feat, best_threshold)};
            stack.push({index * 2 + 1, start, partition_pos});
            stack.push({index * 2 + 2, partition_pos, end});
        }
    }
}

template<size_t ArrSize>
inline float FlatTree::gini_entropy(std::array<int, ArrSize> counts, size_t n)
{
    ZoneScoped;
    if (n == 0) return 0.0;
    auto sum_sq{0.0f};
    for (auto i{0uz}; i < ArrSize; ++i) {
        auto ratio{static_cast<float>(counts[i]) / static_cast<float>(n)};
        sum_sq += ratio * ratio;
    }
    return 1.0 - sum_sq;
}

inline std::pair<int, float> FlatTree::split(size_t index, size_t start, size_t end)
{
    ZoneScoped;
    // could potentially inline. but idt it's necessary
    auto& labels{labels_.get()};
    auto same_label = [&](int s) { return labels[s] == labels[samples_[start]]; };
    is_leaf_[index] = end - start < 5 || std::all_of(samples_.begin() + start, samples_.begin() + end, same_label);
    if (is_leaf_[index]) return {labels[samples_[start]], 0.0f};

    // samples_[start:end] is indices of samples we want
    // copy data[samples_[i]][attr] into buffer, sort, then iterate over splits
    std::vector<std::pair<int, int>> attr_and_label;
    attr_and_label.resize(end - start);

    std::array<int, 100> left_count;
    std::array<int, 100> right_count;

    auto best_score{std::numeric_limits<float>::max()};
    auto best_attr{-1};
    auto best_threshold{0.0f};

    for (size_t attr{0}; attr < num_attr_; ++attr)
    {
        std::fill(left_count.begin(), left_count.end(), 0);
        std::fill(right_count.begin(), right_count.end(), 0);

        for (size_t i{0}; i < attr_and_label.size(); ++i)
        {
            auto attr_val{data_.get()[samples_[start + i]][attr]};
            auto label{labels_.get()[samples_[start + i]]};

            ++right_count[label];
            attr_and_label[i] = {attr_val, label};
        }

        // this one line is taking forever for some reason
        std::sort(attr_and_label.begin(), attr_and_label.end());
        for (size_t i{0}; i < attr_and_label.size(); ++i)
        {
            auto [attr_val, label] = attr_and_label[i];
            if (i > 0 && attr_and_label[i - 1].first != attr_val)
            {
                auto n_left{i};
                auto n_right{attr_and_label.size() - i};
                auto score{n_left * gini_entropy(left_count, n_left) + n_right * gini_entropy(right_count, n_right)};
                if (score < best_score)
                {
                    best_score = score;
                    best_attr = static_cast<int>(attr);
                    best_threshold = (attr_and_label[i - 1].first + attr_val) / 2.0f;
                }
            }
            ++left_count[label];
            --right_count[label];
        }
    }
    return {best_attr, best_threshold};
}

inline size_t FlatTree::hoare_partition(size_t start, size_t end, int feature, float threshold)
{
    ZoneScoped;
    auto value = [&](size_t i) { return data_.get()[samples_[i]][feature]; };
    auto i{start};
    auto j{end - 1};
    while (true)
    {
        while (value(i) <= threshold) ++i;
        while (value(j) > threshold) --j;
        if (i >= j) return i;
        std::swap(samples_[i], samples_[j]);
    }
}

}  // namespace tree
