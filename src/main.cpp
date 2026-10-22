#include "flat_tree.h"
#include "loaders.h"

#include <iostream>

int main() {
    ZoneScoped;
    const auto [rows, targets] = tree::load_car();
    auto ftree{tree::FlatTree(rows, targets)};
    ftree.build_tree();
    for (const auto& row : rows) {
      std::cout << ftree.predict(row) << '\n';
    }
    return 0;
}
