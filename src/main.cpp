//
// Created by disservin on 20.04.2023.
//

#include <iostream>

#include "threadpool.hpp"

ThreadPool pool;

int main() {
    EngineConfig engine_1;

    engine_1.command = "smallbrain.exe";
    engine_1.name = "smallbrain_1";
    engine_1.limit.depth = 7;

    EngineConfig engine_2;

    engine_2.command = "smallbrain.exe";
    engine_2.name = "smallbrain_2";
    engine_2.limit.depth = 7;

    Config config;

    pool.start(std::make_pair(engine_1, engine_2), config, 2);

    // wait for user input
    while (!pool.getStop()) {
        std::string input;
        std::cin >> input;

        if (input == "stop") {
            pool.stop();
        }
    }

    pool.stop();
}