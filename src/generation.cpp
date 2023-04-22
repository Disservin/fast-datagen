//
// Created by disservin on 20.04.2023.
//

#include "generation.hpp"

#include <fstream>
#include <iomanip>
#include <optional>

#include "rand.hpp"
#include "threadpool.hpp"

extern ThreadPool pool;

constexpr int kRandomMoves = 10;

inline std::string Generation::format(const FenData &data, double score) {
    std::ostringstream sstream;
    sstream << data.fen << " [" << std::fixed << std::setprecision(1) << score << "] " << data.score
            << "\n";

    return sstream.str();
}

void Generation::loadBook(const std::string &file_path) {
    if (file_path.empty()) {
        return;
    }

    std::ifstream openingFile;
    std::string line;
    openingFile.open(file_path);

    while (std::getline(openingFile, line)) {
        book_.emplace_back(line);
    }

    openingFile.close();
}

void Generation::init(UciEngine &engine1, UciEngine &engine2, const std::string &file_path) {
    loadBook(file_path);

    engine1.startEngine();
    engine2.startEngine();
}

std::string Generation::fetchFen() {
    if (!book_.empty()) {
        return book_[Random::rand(book_.size() - 1)];
    }
    return randomOpening();
}

std::string Generation::randomOpening() {
    Chess::Board board;
    int ply = 0;

    Chess::Movelist<Chess::Move> moves;
    while (ply < kRandomMoves) {
        Chess::Movegen::legalmoves<Chess::Move, Chess::MoveGenType::ALL>(moves, board);

        if (moves.size() == 0) {
            ply = 0;
            board.loadFen(Chess::STARTPOS);
            continue;
        }

        board.makeMove(moves[static_cast<int>(Random::rand(moves.size() - 1))]);
        ply++;
    }

    return board.getFen();
}

void Generation::randomPlayout(std::ofstream &file, UciEngine &engine1, UciEngine &engine2) {
    Chess::Color winning_color = Chess::Color::NO_COLOR;
    Chess::Board board;
    Chess::Movelist<Chess::Move> moves;

    std::vector<std::string> played_moves;

    engine1.sendUciNewGame();
    engine2.sendUciNewGame();

    win_count_ = 0;
    draw_count_ = 0;

    start_fen_ = fetchFen();
    board.loadFen(start_fen_);

    data_.clear();

    while (true) {
        auto next = playNextMove(engine1, board, played_moves, winning_color);

        if (!next) {
            break;
        }

        next = playNextMove(engine2, board, played_moves, winning_color);
        if (!next) {
            break;
        }
    }

    double score;

    if (winning_color == Chess::Color::WHITE) {
        score = 1.0;
    } else if (winning_color == Chess::Color::BLACK) {
        score = 0.0;
    } else {
        score = 0.5;
    }

    fens_collected_ += data_.size();
    for (const auto &sfen : data_) {
        file << format(sfen, score);
    }
}

bool Generation::playNextMove(UciEngine &engine, Chess::Board &board,
                              std::vector<std::string> &played_moves_,
                              Chess::Color &winning_color) {
    const bool inCheck = board.isKingAttacked();
    const auto gameover = board.isGameOver();

    if (gameover == Chess::GameResult::DRAW) {
        winning_color = Chess::Color::NO_COLOR;
        return false;

    } else if (gameover == Chess::GameResult::LOSE) {
        winning_color = ~board.sideToMove();
        return false;
    }

    engine.sendGo(start_fen_, played_moves_);

    bool timeout = false;
    auto output = engine.readEngine("bestmove", timeout, 0);

    const auto bestMove = engine.bestmove();
    const auto score_type = engine.lastScoreType();
    const auto score = engine.lastScore();
    const auto move = board.uciToMove(bestMove);

    updateCounts(score_type, score);

    auto adjudication = shouldAdjudicate(board, score);

    if (adjudication.first) {
        winning_color = adjudication.second;
        return false;
    }

    const bool isCapture = board.pieceAt(move.to()) != Chess::Piece::NONE;

    auto add = shouldRecord(engine, inCheck, isCapture, kRandomMoves, score_type, score);

    if (add) {
        data_.emplace_back(board.getFen(),
                           score * (board.sideToMove() == Chess::Color::WHITE ? 1 : -1));
    }

    played_moves_.emplace_back(bestMove);

    board.makeMove(move);

    return true;
}

bool Generation::shouldRecord(const UciEngine &engine, bool inCheck, bool isCapture, int ply,
                              std::string_view score_type, int score) {
    if (inCheck) {
        return false;
    }

    if (ply < 10) {
        return false;
    }

    if (score_type == "mate") {
        return false;
    }

    if (std::abs(score) > 1500) {
        return false;
    }

    if (isCapture) {
        return false;
    }

    return true;
}

void Generation::updateCounts(const std::string &score_type, int score) {
    if (std::abs(score) > 1500 || score_type == "mate") {
        win_count_++;
        draw_count_ = 0;
    } else if (std::abs(score) <= 4 && score_type == "cp") {
        win_count_ = 0;
        draw_count_++;
    } else {
        win_count_ = 0;
        draw_count_ = 0;
    }
}

std::pair<bool, Chess::Color> Generation::shouldAdjudicate(const Chess::Board &board,
                                                           int score) const {
    if (win_count_ >= 4) {
        auto win = score > 0 ? board.sideToMove() : ~board.sideToMove();
        return {true, win};
    } else if (draw_count_ >= 6) {
        return {true, Chess::Color::NO_COLOR};
    }

    return {false, Chess::Color::NO_COLOR};
}

void Generation::start(std::pair<EngineConfig, EngineConfig> engines, const std::string &file_path,
                       int thread_id) {
    UciEngine engine1 = UciEngine(engines.first.command);
    UciEngine engine2 = UciEngine(engines.second.command);

    engine1.loadConfig(engines.first);
    engine2.loadConfig(engines.second);

    std::ofstream file;
    file.open("data/data" + std::to_string(thread_id) + ".txt", std::ios::app);

    init(engine1, engine2, file_path);

    auto t0 = std::chrono::high_resolution_clock::now();

    uint64_t games = 0;
    while (!pool.getStop()) {
        games++;
        randomPlayout(file, engine1, engine2);

        if (thread_id == 0 && games % 20 == 0) {
            auto t1 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count();

            std::cout << "\r" << std::setw(10) << double(games) / double(duration + 1)
                      << " games/s " << double(fens_collected_) / double(duration + 1)
                      << " fens/s elapsed:" << std::setw(10) << duration << " seconds. "
                      << std::setw(10) << fens_collected_ << " fens collected. " << std::flush;
        }
    }

    file.flush();

    file.close();
}
