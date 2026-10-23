#include <vector>
#include <filesystem>

#include "csv.h"

namespace tree
{

template <typename T = int, typename L = int>
inline std::pair<std::vector<std::vector<T>>, std::vector<L>> load_car()
{
    const auto path = std::filesystem::path{TREE_RACE_SOURCE_DIR} / "datasets" / "car_eval.csv";
    io::CSVReader<7> in(path.string());
    std::array<T, 7> a;
    std::vector<std::vector<T>> row_data;
    std::vector<L> target_data;
    while (in.read_row(a[0], a[1], a[2], a[3], a[4], a[5], a[6]))
    {
        row_data.push_back({a[0], a[1], a[2], a[3], a[4], a[5]});
        target_data.push_back(a[6]);
    }
    return {std::move(row_data), std::move(target_data)};
}

}  // namespace tree
