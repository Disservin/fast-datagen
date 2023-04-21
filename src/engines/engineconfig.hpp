//
// Created by disservin on 20.04.2023.
//

#pragma once

#include <string>

struct EngineLimit {
    int nodes = 0;
    int depth = 0;
};

struct EngineConfig {
    std::string name;
    std::string command;
    EngineLimit limit;
};
