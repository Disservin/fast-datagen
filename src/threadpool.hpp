//
// Created by disservin on 20.04.2023.
//

#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "engines/engineconfig.hpp"
#include "generation.hpp"

struct Config {
    std::string file_path;
};

class ThreadPool {
   public:
    ThreadPool() = default;
    ~ThreadPool() = default;

    inline void start(std::pair<EngineConfig, EngineConfig> engines, Config configs, int count);
    inline void stop();

    inline bool getStop() { return stop_; }

   private:
    std::vector<std::thread> runningThreads;

    std::atomic_bool stop_{};
};

void ThreadPool::start(std::pair<EngineConfig, EngineConfig> engines, Config configs, int count) {
    stop_ = false;

    for (int i = 0; i < count; i++) {
        runningThreads.emplace_back(&Generation::start, Generation(), engines, configs.file_path,
                                    i);
    }
}

void ThreadPool::stop() {
    stop_ = true;

    for (auto &thread : runningThreads) {
        thread.join();
    }
}
