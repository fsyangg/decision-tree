#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <stack>
#include <tracy/Tracy.hpp>
#include <vector>
#if TREE_SIMD
#include <arm_neon.h>
#endif

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

#if TREE_SIMD
    static constexpr uint16_t iota_vec[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    float32_t gini_entropy(uint16x8_t counts, uint16_t n);
    uint16x8_t inc_u16x8(uint16x8_t v, uint16_t idx);

    uint16_t cmb_u8(uint8_t lo, uint8_t hi);
    std::pair<uint8_t, uint8_t> sep_u16(uint16_t x);
#else
    template <size_t ArrSize>
    float gini_entropy(const std::array<int, ArrSize>& counts, size_t n);
#endif

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

#if TREE_SIMD
inline float32_t FlatTree::gini_entropy(uint16x8_t counts, uint16_t n)
{
    ZoneScopedN("simd-gini");
    float16x8_t num = vcvtq_f16_u16(counts);
    float16x8_t denom = vcvtq_f16_u16(vdupq_n_u16(n));

    float16x8_t res = vdivq_f16(num, denom);
    res = vmulq_f16(res, res);

    // no horizontal f16 add intrinsic for neon, need to convert to two float32x4_t
    float32x4_t hi = vcvt_high_f32_f16(res);
    float32x4_t lo = vcvt_f32_f16(vget_low_f16(res));
    return static_cast<float32_t>(1.0) - vaddvq_f32(vaddq_f32(lo, hi));
}

inline uint16x8_t FlatTree::inc_u16x8(uint16x8_t v, uint16_t idx)
{
    // v[idx] - 0xFFFF
    uint16x8_t iota = vld1q_u16(iota_vec);
    uint16x8_t mask = vceqq_u16(iota, vdupq_n_u16(static_cast<uint16_t>(idx)));
    return vsubq_u16(v, mask);
}
#else
template <size_t ArrSize>
inline float FlatTree::gini_entropy(const std::array<int, ArrSize>& counts, size_t n)
{
    ZoneScopedN("non-simd-gini");
    if (n == 0) return 0.0;
    auto sum_sq{0.0f};
    for (auto i{0uz}; i < ArrSize; ++i)
    {
        auto ratio{static_cast<float>(counts[i]) / static_cast<float>(n)};
        sum_sq += ratio * ratio;
    }
    return 1.0 - sum_sq;
}
#endif

inline uint16_t FlatTree::cmb_u8(uint8_t lo, uint8_t hi) {
    return static_cast<uint16_t>(lo) | (static_cast<uint16_t>(hi) << 8);
}

inline std::pair<uint8_t, uint8_t> FlatTree::sep_u16(uint16_t x) {
    return {static_cast<uint8_t>(x & 0x00FF), static_cast<uint8_t>(x >> 8)};
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

    std::vector<uint16_t> attr_and_label;
    attr_and_label.resize(end - start);

#if !TREE_SIMD
    std::array<int, 100> left_count;
    std::array<int, 100> right_count;
#endif

    auto best_score{std::numeric_limits<float>::max()};
    auto best_attr{static_cast<uint8_t>(0)};
    auto best_threshold{0.0f};

    for (size_t attr{0}; attr < num_attr_; ++attr)
    {
#if TREE_SIMD
        uint16x8_t total_count = vdupq_n_u16(0);
        uint16x8_t left_count = vdupq_n_u16(0);
#else
        std::fill(left_count.begin(), left_count.end(), 0);
        std::fill(right_count.begin(), right_count.end(), 0);
#endif

        for (size_t i{0}; i < attr_and_label.size(); ++i)
        {
            auto attr_val{static_cast<uint8_t>(data_.get()[samples_[start + i]][attr])};
            auto label{static_cast<uint8_t>(labels_.get()[samples_[start + i]])};

#if TREE_SIMD
            total_count = inc_u16x8(total_count, label);
#else
            ++right_count[label];
#endif
            attr_and_label[i] = cmb_u8(label, attr_val);
        }

        std::sort(attr_and_label.begin(), attr_and_label.end(),
                [](uint16_t a, uint16_t b){ return (a >> 8) < (b >> 8); });
        auto old_val{static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())};
        for (size_t i{0}; i < attr_and_label.size(); ++i)
        {
            auto [label, attr_val] = sep_u16(attr_and_label[i]);
            if (old_val != attr_val)
            {
                auto n_left{i};
                auto n_right{attr_and_label.size() - i};
#if TREE_SIMD
                auto left{n_left * gini_entropy(left_count, n_left)};
                auto right{n_right * gini_entropy(vsubq_u16(total_count, left_count), n_right)};
                auto score{left + right};
#else
                auto score{n_left * gini_entropy(left_count, n_left) + n_right * gini_entropy(right_count, n_right)};
#endif
                if (score < best_score)
                {
                    best_score = score;
                    best_attr = static_cast<int>(attr);
                    best_threshold = static_cast<float>(old_val + attr_val) / 2.0f;
                }
                old_val = attr_val;
            }
#if TREE_SIMD
            left_count = inc_u16x8(left_count, label);
#else
            ++left_count[label];
            --right_count[label];
#endif
        }
    }
    return {static_cast<int>(best_attr), static_cast<float>(best_threshold)};
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
