#pragma once
#include <ncurses.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
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
    static const int WIDTH = 10;
    static const int HEIGHT = 22;
    static const int VISIBLE_HEIGHT = 20;

    struct Piece
    {
        int shape, rot, x, y;
    };

    int field[HEIGHT][WIDTH];
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

    static const int piece_color_ids[8];
    static const char tetromino[7][4][4][4];

    void initColors256();
    bool check(const Piece &p) const;
    void merge(const Piece &p);
    int clearLines();
    void drawBoard() const;
    void drawInfo() const;
    void spawnPiece();
    void handleInput(int ch);
    void applyGravity(int ch);
    void gameOver();
    void refillBag(); // For improved randomization
    // Function to Find Ghost Position
    Piece getGhostPiece() const;
    // Yes or no prompt function
    bool confirmAction(const std::string &prompt);

    // Highscore fie-handling
    void loadHighscore();
    void saveHighscore();
};
