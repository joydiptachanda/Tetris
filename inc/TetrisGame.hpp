#pragma once
#include <queue>
#include <random>
#include <string>
#include "Tetromino.hpp"
#include "Board.hpp"
#include "Renderer.hpp"
#include "HighscoreManager.hpp"
#include "Logger.hpp"

class TetrisGame
{
public:
    TetrisGame();
    ~TetrisGame();
    void run();

private:
    static constexpr int WIDTH = BOARD_WIDTH;
    static constexpr int HEIGHT = BOARD_HEIGHT;
    static constexpr int VISIBLE_HEIGHT = BOARD_VISIBLE_HEIGHT;

    Board board;
    Renderer renderer;
    HighscoreManager highscore;
    std::mt19937 rng{std::random_device{}()};

    int score, level, delay, frame;
    bool running;
    // pause feature
    bool paused;
    Piece curr{}, next{};
    std::queue<int> pieceQueue; // For improved randomization
    // Implement "Hold Piece" Feature
    Piece hold{};
    bool holdUsedThisTurn;
    bool holding;

    bool hardDropped;

    bool boardDirty = true;
    bool infoDirty = true;

    void spawnPiece();
    void restartGame();
    void redrawAll();

    void handleInput(int ch);
    // Helpers for handleInput breakdown:
    bool handlePauseKey(int ch);
    bool handleHoldKey(int ch);
    bool handleClearHighscoreKey(int ch);
    bool handleQuitKey(int ch);
    bool handleRestartKey(int ch);
    bool handleMoveKey(int ch, Piece &temp);
    bool handleRotateKey(int ch, Piece &temp);
    bool handleDropKey(int ch, Piece &temp);

    void applyGravity();
    void awardScoreAndLevel(int lines);

    void gameOver();
    void refillBag();

    // Yes or no prompt function
    bool confirmAction(const std::string &prompt);
    int waitForKey(); // blocks (via short sleeps) until a key is available
};
