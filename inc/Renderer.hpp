#pragma once
#include <ncursesw/ncurses.h>
#include <string>
#include "Tetromino.hpp"
#include "Board.hpp"

// Owns the ncurses windows and all drawing code; never mutates game state.
class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    // Must be called after initscr().
    void init();
    void initColors();

    void drawBoard(const Board &board, const Piece &curr);
    void drawInfo(int score, int level, const std::string &highscoreName, int highscoreScore,
                  const Piece &next, bool holding, const Piece &hold, bool paused);

    void drawGameOverScreen();
    void drawHighscorePrompt();
    void drawConfirmActionScreen(const std::string &prompt);

private:
    WINDOW *gameWin = nullptr;
    WINDOW *sideWin = nullptr;

    void drawCells(const Board &board, const Piece &curr,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isGhostCell,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isCurrCell);
    void drawScorePanel(int score, int level, const std::string &highscoreName, int highscoreScore);
    void drawNextPreview(const Piece &next);
    void drawHoldPreview(bool holding, const Piece &hold);
    void drawControls();
    void drawPauseState(bool paused);
};
