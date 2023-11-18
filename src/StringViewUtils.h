#include <string_view>
#include <array>
#include <optional>
#include <string>

template <std::size_t N>
void Append(std::array<char, N> &array, std::string_view what)
{
    auto const alreadyContains = std::string_view{array.data()};
    auto const remaining = array.size() - alreadyContains.size();
    std::memcpy(std::next(array.data(), alreadyContains.size()), what.data(), std::min(what.size(), remaining));
}

template <std::size_t N>
void Append(std::array<char, N> &array, int what)
{
    auto const alreadyContains = std::string_view{array.data()};
    auto const remaining = array.size() - alreadyContains.size();
    snprintf(std::next(array.data(), alreadyContains.size()), remaining, "%d", what);
}

template <std::size_t N1, std::size_t N2>
void split(std::string_view string, std::array<std::array<char, N1>, N2> &array, char delimiter = ',')
{
    for (auto &item : array)
    {
        std::fill(item.begin(), item.end(), 0);
    }

    size_t pos_start = 0, pos_end, delim_len = 1U;
    std::size_t indexOfElement = 0U;

    while ((pos_end = string.find(delimiter, pos_start)) != std::string::npos)
    {
        auto token = string.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        if (indexOfElement < array.size())
        {
            std::memcpy(array.at(indexOfElement).data(), token.data(), std::min(token.size(), array.at(indexOfElement).size()));
        }
        ++indexOfElement;
    }

    if (indexOfElement < array.size())
    {
        auto const token = string.substr(pos_start);
        std::memcpy(array.at(indexOfElement).data(), token.data(), std::min(token.size(), array.at(indexOfElement).size()));
    }
    ++indexOfElement;
}

std::vector<std::string> split_as_vector(std::string_view string, char delimiter = ',')
{
    auto result = std::vector<std::string>{};

    size_t pos_start = 0, pos_end, delim_len = 1U;

    while ((pos_end = string.find(delimiter, pos_start)) != std::string::npos)
    {
        auto token = string.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        result.emplace_back(std::string{token});
    }
    auto const token = string.substr(pos_start);
    result.emplace_back(std::string{token});

    return result;
}

template <typename T>
std::optional<T> parseTo(std::string_view string)
{
    if (string.size() == 0)
    {
        return std::optional<T>{};
    }

    if constexpr (std::is_same_v<double, T>)
    {
        auto const result = strtod(string.data(), NULL);
        return result == 0 && string.front() != '0' ? std::nullopt : std::make_optional(result);
    }
    else
    {
        auto const result = atoi(string.data());
        return result == 0 && string.front() != '0' ? std::nullopt : std::make_optional(result);
    }
}