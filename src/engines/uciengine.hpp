//
// Created by disservin on 20.04.2023.
//

#pragma once

#include "engines/engineconfig.hpp"
#include "engines/engineprocess.hpp"

class UciEngine : EngineProcess {
   public:
    explicit UciEngine(const std::string &command) : EngineProcess(command){};
    ~UciEngine() { sendQuit(); }

    std::vector<std::string> readUci();

    void sendUciNewGame();
    void sendUci();
    void sendQuit();

    bool isResponsive(int64_t threshold = ping_time_);

    void loadConfig(const EngineConfig &config);
    [[nodiscard]] EngineConfig getConfig() const;

    static std::string buildPositionInput(const std::string &fen,
                                          const std::vector<std::string> &moves);
    [[nodiscard]] std::string buildGoInput() const;

    void sendGo(const std::string &fen, const std::vector<std::string> &moves);

    void startEngine();

    const std::vector<std::string> &readEngine(std::string_view last_word, bool &timeout,
                                               int64_t timeoutThreshold = 1000);
    void writeEngine(const std::string &input);

    std::string bestmove() const;
    std::vector<std::string> lastInfo() const;
    std::string lastScoreType() const;
    int lastScore() const;

    static const int64_t ping_time_ = 60000;

   private:
    std::vector<std::string> output_;

    EngineConfig config_;
};