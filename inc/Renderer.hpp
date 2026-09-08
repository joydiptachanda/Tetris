// =============================================================================
// Renderer.hpp
// -----------------------------------------------------------------------------
// Owns the terminal and all drawing/input code; never mutates game state.
// The only class that talks to Terminal.
// =============================================================================
#pragma once
#include <array>
#include <string>
#include "Tetromino.hpp"
#include "Board.hpp"
#include "Terminal.hpp"

/**
 * @brief Owns the terminal and all drawing/input code; never mutates game state.
 *
 * @details
 * The only class in the codebase that talks to Terminal.
 */
class Renderer
{
public:
    Renderer();

    /**
     * @brief Draws the game board border and cells.
     * @param [in] board      Grid to render.
     * @param [in] curr       Currently falling piece (used for ghost/current cell styling).
     * @param [in] hiddenRows hiddenRows[i]==true blanks out row i's filled cells;
     *                        used for the line-clear blink animation.
     */
    void drawBoard(const Board &board, const Piece &curr,
                   const std::array<bool, BOARD_HEIGHT> &hiddenRows = {});

    /**
     * @brief Draws the side panel: score, next/hold previews, controls, and pause state.
     * @param [in] score           Current score.
     * @param [in] level           Current level.
     * @param [in] highscoreName   Highscore holder's name.
     * @param [in] highscoreScore  Highscore value.
     * @param [in] next            Next piece to spawn.
     * @param [in] holding         Whether a piece is currently held.
     * @param [in] hold            The held piece, if holding is true.
     * @param [in] paused          Whether the game is currently paused.
     */
    void drawInfo(int score, int level, const std::string &highscoreName, int highscoreScore,
                  const Piece &next, bool holding, const Piece &hold, bool paused);

    /** @brief Flushes everything drawn since the last present() to the terminal in one write. */
    void present();

    /** @brief Draws the "GAME OVER" overlay. */
    void drawGameOverScreen();
    /** @brief Draws the "NEW HIGHSCORE! Enter name:" overlay. */
    void drawHighscorePrompt();

    /**
     * @brief Draws a yes/no confirmation overlay.
     * @param [in] prompt Prompt text to display (truncated to fit the box).
     */
    void drawConfirmActionScreen(const std::string &prompt);

    /** @brief Erases the confirm/game-over/highscore prompt area. */
    void clearOverlay();

    /**
     * @brief Reads a bounded line of text at the highscore-prompt input position.
     * @param [in] maxLen Maximum number of characters accepted.
     * @return The entered name (may be empty).
     * @details
     * Handles backspace/enter locally since raw mode disables terminal echo.
     */
    std::string promptHighscoreName(size_t maxLen);

    /**
     * @brief Reads the next queued key.
     * @retval Terminal::None Nothing is waiting; returns immediately (non-blocking).
     * @retval other          The next available key code.
     */
    int pollKey();

    /** @brief Discards any currently buffered input. */
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

    /**
     * @brief Moves the cursor and writes text.
     * @param [in] row  Target row.
     * @param [in] col  Target column.
     * @param [in] text Text to write.
     */
    void put(int row, int col, const std::string &text);

    /**
     * @brief Moves the cursor and writes a printf-style formatted string.
     * @param [in] row Target row.
     * @param [in] col Target column.
     * @param [in] fmt printf-style format string, followed by its arguments.
     */
    void putf(int row, int col, const char *fmt, ...);

    /** @brief Draws the game board's outer border. */
    void drawGameBorder();
    /** @brief Draws the side panel's outer border. */
    void drawSideBorder();

    /**
     * @brief Draws every board cell, applying ghost/current/hidden-row styling.
     * @param [in] board       Grid to render.
     * @param [in] curr        Currently falling piece.
     * @param [in] isGhostCell Per-cell mask of the ghost piece's projected position.
     * @param [in] isCurrCell  Per-cell mask of the current piece's position.
     * @param [in] hiddenRows  Rows to render blank (line-clear blink animation).
     */
    void drawCells(const Board &board, const Piece &curr,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isGhostCell,
                   const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isCurrCell,
                   const std::array<bool, BOARD_HEIGHT> &hiddenRows);

    /** @brief Draws the score/level/highscore lines. */
    void drawScorePanel(int score, int level, const std::string &highscoreName, int highscoreScore);
    /** @brief Draws the "NEXT" piece preview box. */
    void drawNextPreview(const Piece &next);
    /** @brief Draws the "HOLD" piece preview box. */
    void drawHoldPreview(bool holding, const Piece &hold);
    /** @brief Draws the controls legend. */
    void drawControls();
    /** @brief Draws the "PAUSED" banner when paused. */
    void drawPauseState(bool paused);
};
