//
// Created by disservin on 20.04.2023.
//

#pragma once

#include <random>

namespace Random {

static std::random_device rd_;
static std::mt19937 rng(rd_());

size_t rand(size_t max) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, max);
    return dist(rng);
}
}  // namespace Random