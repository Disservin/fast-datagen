//
// Created by mcall on 21.04.2023.
//

#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

template <typename T>
std::optional<T> findElement(const std::vector<std::string> &haystack, std::string_view needle) {
    auto position = std::find(haystack.begin(), haystack.end(), needle);
    auto index = position - haystack.begin();
    if (position == haystack.end()) return std::nullopt;
    if constexpr (std::is_same_v<T, int>)
        return std::stoi(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, float>)
        return std::stof(haystack[index + 1]);
    else if constexpr (std::is_same_v<T, uint64_t>)
        return std::stoull(haystack[index + 1]);
    else
        return haystack[index + 1];
}

inline std::vector<std::string> splitString(const std::string &string, const char &delimiter) {
    std::stringstream string_stream(string);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(string_stream, segment, delimiter)) seglist.emplace_back(segment);

    return seglist;
}