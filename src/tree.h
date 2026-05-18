#pragma once

#include <vector>

namespace DTree {

using Data = std::pair<std::vector<std::vector<int>>, std::vector<int>>;
using Tree = void;

class DTree
{
public:

    DTree() = default;

    // inline gini()

    Tree train(const Data& data) {
        // consider each feature
        // consider each threshold for each feature, and

    }

    int inference(const std::vector<int>& f_row) {
        return -1;
    }

private:

};

} // namespace DTree
