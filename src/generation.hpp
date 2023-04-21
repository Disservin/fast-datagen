//
// Created by disservin on 20.04.2023.
//

#pragma once

#include <fstream>
#include <memory>
#include <vector>

#include "engines/engineconfig.hpp"
#include "engines/uciengine.hpp"
#include "third_party/chess-library/src/chess.hpp"

struct FenData {
    std::string fen;
    int score;

    FenData(std::string _fen, int _score) : fen(std::move(_fen)), score(_score) {}
};

class Generation {
   public:
    void start(std::pair<EngineConfig, EngineConfig> engines, const std::string &file_path,
               int thread_id);

   private:
    void init(UciEngine &engine1, UciEngine &engine2, const std::string &file_path);

    static std::string format(const FenData &data, double score);

    void loadBook(const std::string &file_path);

    static std::string randomOpening();
    std::string fetchFen();

    void updateCounts(const std::string &score_type, int score);

    static bool shouldRecord(UciEngine &engine, bool inCheck, bool isCapture, int ply,
                             std::string_view score_type, int score);
    std::pair<bool, Chess::Color> shouldAdjudicate(Chess::Board &board, int score) const;

    bool playNextMove(UciEngine &engine, Chess::Board &board,
                      std::vector<std::string> &played_moves, Chess::Color &winning_color);

    void randomPlayout(std::ofstream &file, UciEngine &engine1, UciEngine &engine2);

    std::vector<FenData> data_;
    std::vector<std::string> book_;

    std::pair<EngineConfig, EngineConfig> engines_;

    std::string start_fen_;

    uint64_t fens_collected_ = 0;

    int win_count_ = 0;
    int draw_count_ = 0;
};
