#include "csv.h"
#include "flat_tree.h"

#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

using Data = std::pair<std::vector<std::vector<int>>, std::vector<int>>;

Data load_tennis_data() {
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

    return {std::move(row_data), std::move(target_data)};
}

int main() {
    const auto [rows, targets] = load_tennis_data();
    std::cout << "loaded tennis rows: " << rows.size()
        << ", labels: " << targets.size() << '\n';

    auto ftree{tree::FlatTree(rows, targets)};
    for (int i{0}; i < 1'000'000; ++i) {
        ftree.build_tree();
    }
    auto i{ftree.predict(rows[0])};
    std::cout << "done" << '\n';
}
