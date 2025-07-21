#pragma once
#include <ncurses.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <array>
#include <sstream>
#include <fstream>
#include "Logger.hpp"

class TetrisGame
{
public:
    TetrisGame();
    ~TetrisGame();
    void run();

private:
    static constexpr int WIDTH = 10;
    static constexpr int HEIGHT = 22;
    static constexpr int VISIBLE_HEIGHT = 20;

    struct Piece
    {
        int shape, rot, x, y;
    };

    std::array<std::array<int, WIDTH>, HEIGHT> field{};

    int score, level, delay, frame;
    bool running;
    // pause feature
    bool paused;
    WINDOW *gameWin, *sideWin;
    Piece curr, next;
    std::queue<int> pieceQueue; // For improved randomization
    // Implement "Hold Piece" Feature
    Piece hold;
    bool holdUsedThisTurn;
    bool holding;

    // Highscore variable
    std::string highscore_name;
    int highscore_score;
    bool hardDropped;

    bool boardDirty = true;
    bool infoDirty = true;

    // Use std::array for color and tetromino data
    static const std::array<int, 8> piece_color_ids;
    static const std::array<std::array<std::array<std::array<char, 4>, 4>, 4>, 7> tetromino;

    void initColors256();
    bool check(const Piece &p) const;
    void merge(const Piece &p);
    int clearLines();

    void drawBoard() const;
    std::array<std::array<bool, WIDTH>, HEIGHT> computeGhostMask() const;
    std::array<std::array<bool, WIDTH>, HEIGHT> computeCurrentMask() const;
    void drawCells(
        const std::array<std::array<bool, WIDTH>, HEIGHT> &isGhostCell,
        const std::array<std::array<bool, WIDTH>, HEIGHT> &isCurrCell) const;

    void drawInfo() const;
    void drawScorePanel() const;
    void drawNextPreview() const;
    void drawHoldPreview() const;
    void drawControls() const;
    void drawPauseState() const;

    void spawnPiece();

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

    void applyGravity(int ch);
    void awardScoreAndLevel(int lines);

    void gameOver();
    void refillBag();
    Piece getGhostPiece() const;
    // Yes or no prompt function
    bool confirmAction(const std::string &prompt);

    // Highscore fie-handling
    void loadHighscore();
    void saveHighscore();
};
