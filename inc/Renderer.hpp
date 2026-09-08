#pragma once
#include <array>
#include <string>
#include "Tetromino.hpp"
#include "Board.hpp"
#include "Terminal.hpp"

// Owns the terminal and all drawing/input code; never mutates game state.
class Renderer
{
public:
    Renderer();

    // hiddenRows[i]==true blanks out row i's filled cells (used for the line-clear blink).
    void drawBoard(const Board &board, const Piece &curr,
                   const std::array<bool, BOARD_HEIGHT> &hiddenRows = {});
    void drawInfo(int score, int level, const std::string &highscoreName, int highscoreScore,
                  const Piece &next, bool holding, const Piece &hold, bool paused);
    void present(); // flushes everything drawn since the last present()

    void drawGameOverScreen();
    void drawHighscorePrompt();
    void drawConfirmActionScreen(const std::string &prompt);
    void clearOverlay(); // erases the confirm/game-over/highscore prompt area

    // Reads a bounded line of text at the highscore-prompt input position,
    // handling backspace/enter locally since raw mode disables terminal echo.
    std::string promptHighscoreName(size_t maxLen);

    // Input passthrough: Terminal owns all raw-mode/ANSI I/O for the game.
    int pollKey();
    void flushInput();

private:
    Terminal term;
    int nameInputRow = 0, nameInputCol = 0;

    // Absolute (1-indexed) screen origins, mirroring the old ncurses window
    // positions: gameWin begin_y=1,begin_x=2 and sideWin begin_y=1,begin_x=WIDTH*2+4.
    static constexpr int GAME_BEGIN_Y = 1;
    static constexpr int GAME_BEGIN_X = 2;
    static constexpr int SIDE_BEGIN_Y = 1;
    static constexpr int SIDE_BEGIN_X = BOARD_WIDTH * 2 + 4;
    static constexpr int GAME_ROWS = BOARD_VISIBLE_HEIGHT + 2;
    static constexpr int GAME_COLS = BOARD_WIDTH * 2 + 2;
    static constexpr int SIDE_ROWS = BOARD_VISIBLE_HEIGHT + 2;
    static constexpr int SIDE_COLS = 35;

    static int gameRow(int y) { return GAME_BEGIN_Y + y + 1; }
    static int gameCol(int x) { return GAME_BEGIN_X + x + 1; }
    static int sideRow(int y) { return SIDE_BEGIN_Y + y + 1; }
    static int sideCol(int x) { return SIDE_BEGIN_X + x + 1; }

    void put(int row, int col, const std::string &text);
    void putf(int row, int col, const char *fmt, ...);

    void drawGameBorder();
    void drawSideBorder();
    void drawCells(const Board &board, const Piece &curr,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isGhostCell,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isCurrCell,
                   const std::array<bool, BOARD_HEIGHT> &hiddenRows);
    void drawScorePanel(int score, int level, const std::string &highscoreName, int highscoreScore);
    void drawNextPreview(const Piece &next);
    void drawHoldPreview(bool holding, const Piece &hold);
    void drawControls();
    void drawPauseState(bool paused);
};
