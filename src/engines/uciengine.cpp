//
// Created by disservin on 20.04.2023.
//

#include "engines/uciengine.hpp"

#include <cassert>
#include <iostream>

#include "helper.h"

std::vector<std::string> UciEngine::readUci() {
    bool timeout = false;
    return readEngine("uciok", timeout);
}

void UciEngine::sendUciNewGame() { writeEngine("ucinewgame"); }
void UciEngine::sendUci() { writeEngine("uci"); }
void UciEngine::sendQuit() { writeEngine("quit"); }

bool UciEngine::isResponsive(int64_t threshold) {
    writeEngine("isready");
    bool timeout = false;
    readEngine("readyok", timeout, threshold);
    return !timeout;
}

void UciEngine::loadConfig(const EngineConfig &config) { config_ = config; }

EngineConfig UciEngine::getConfig() const { return config_; }

std::string UciEngine::buildPositionInput(const std::string &fen,
                                          const std::vector<std::string> &moves) {
    std::string position = "position fen " + fen;

    if (!moves.empty()) {
        position += " moves";
        for (const auto &move : moves) {
            position += " " + move;
        }
    }

    return position;
}

std::string UciEngine::buildGoInput() const {
    if (config_.limit.nodes > 0) {
        return "go nodes " + std::to_string(config_.limit.nodes);
    }

    if (config_.limit.depth > 0) {
        return "go depth " + std::to_string(config_.limit.depth);
    }

    assert(false);
    return "go";
}

void UciEngine::sendGo(const std::string &fen, const std::vector<std::string> &moves) {
    writeEngine(buildPositionInput(fen, moves));
    writeEngine(buildGoInput());
}

void UciEngine::startEngine() {
    sendUci();
    readUci();

    if (!isResponsive(60000)) {
        throw std::runtime_error("Warning: Something went wrong when pinging the engine.");
    }

    sendUciNewGame();
}

const std::vector<std::string> &UciEngine::readEngine(std::string_view last_word, bool &timeout,
                                                      int64_t timeoutThreshold) {
    try {
        output_.clear();
        output_ = readProcess(last_word, timeout, timeoutThreshold);
        return output_;
    } catch (const std::exception &e) {
        std::cout << "Raised Exception in readProcess\nWarning: Engine " << config_.name
                  << " disconnects\n";
        throw e;
    }
}

void UciEngine::writeEngine(const std::string &input) {
    try {
        writeProcess(input);
    } catch (const std::exception &e) {
        std::cout << "Raised Exception in writeProcess\nWarning: Engine " << config_.name
                  << " disconnects\n";
        throw e;
    }
}

std::string UciEngine::bestmove() const {
    return findElement<std::string>(splitString(output_.back(), ' '), "bestmove").value();
}

std::vector<std::string> UciEngine::lastInfo() const {
    return splitString(output_[output_.size() - 2], ' ');
}

std::string UciEngine::lastScoreType() const {
    return findElement<std::string>(lastInfo(), "score").value_or("cp");
}

int UciEngine::lastScore() const {
    return findElement<int>(lastInfo(), lastScoreType()).value_or(0);
}
