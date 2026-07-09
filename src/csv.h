#pragma once

#include <array>
#include <charconv>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace io
{
template <std::size_t Columns>
class CSVReader
{
  public:
    explicit CSVReader(const std::string &filename) : input_(filename)
    {
        if (!input_)
        {
            throw std::runtime_error("failed to open CSV file: " + filename);
        }
    }

    template <typename... Args>
    bool read_row(Args &...args)
    {
        static_assert(sizeof...(Args) == Columns, "read_row argument count must match CSVReader column count");

        std::string line;
        if (!std::getline(input_, line))
        {
            return false;
        }

        std::array<int, Columns> values{};
        auto *begin = line.data();
        const auto *const end = line.data() + line.size();

        for (std::size_t column = 0; column < Columns; ++column)
        {
            auto *token_end = begin;
            while (token_end < end && *token_end != ',')
            {
                ++token_end;
            }

            if (token_end > begin && token_end[-1] == '\r')
            {
                --token_end;
            }

            const auto result = std::from_chars(begin, token_end, values[column]);
            if (result.ec != std::errc{} || result.ptr != token_end)
            {
                throw std::runtime_error("failed to parse integer CSV field");
            }

            begin = token_end + (token_end < end && *token_end == ',' ? 1 : 0);
        }

        assign(values, args...);
        return true;
    }

  private:
    template <typename... Args>
    static void assign(const std::array<int, Columns> &values, Args &...args)
    {
        assign_impl(values, std::index_sequence_for<Args...>{}, args...);
    }

    template <std::size_t... Indices, typename... Args>
    static void assign_impl(const std::array<int, Columns> &values, std::index_sequence<Indices...>, Args &...args)
    {
        ((args = values[Indices]), ...);
    }

    std::ifstream input_;
};
} // namespace io
