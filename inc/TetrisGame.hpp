// =============================================================================
// TetrisGame.hpp
// -----------------------------------------------------------------------------
// Orchestrates a single game session: the main loop, input dispatch,
// gravity/scoring, and piece spawning. Delegates grid logic to Board,
// all drawing/input to Renderer, and persistence to HighscoreManager.
// =============================================================================
#pragma once
#include <queue>
#include <random>
#include <string>
#include <vector>
#include "Tetromino.hpp"
#include "Board.hpp"
#include "Renderer.hpp"
#include "HighscoreManager.hpp"
#include "Logger.hpp"

/**
 * @brief Orchestrates a single game session.
 *
 * @details
 * Runs the main loop, dispatches input, drives gravity/scoring, and spawns
 * pieces. Delegates grid logic to Board, all drawing/input to Renderer, and
 * persistence to HighscoreManager.
 */
class TetrisGame
{
public:
    /** @brief Sets up game state and takes over the terminal (via Renderer). */
    TetrisGame();
    ~TetrisGame();

    // Owns a live terminal session (via Renderer/Terminal); not copyable or movable.
    TetrisGame(const TetrisGame &) = delete;
    TetrisGame &operator=(const TetrisGame &) = delete;
    TetrisGame(TetrisGame &&) = delete;
    TetrisGame &operator=(TetrisGame &&) = delete;

    /** @brief Runs the main game loop until the player quits. */
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

    // Line-clear blink animation: briefly toggles the cleared row(s) on/off
    // before actually removing them, matching the classic Tetris blink effect.
    static constexpr int BLINK_PERIOD_FRAMES = 3; // frames per on/off phase
    static constexpr int BLINK_TOGGLES = 4;       // on,off,on,off
    bool clearingLines = false;
    std::vector<int> clearingRows;
    int clearAnimFrame = 0;

    bool boardDirty = true;
    bool infoDirty = true;

    void spawnPiece();
    void restartGame();
    void redrawAll();

    /**
     * @brief Dispatches a key to the appropriate handle*Key() helper below.
     * @param [in] ch Key code returned by Renderer::pollKey().
     */
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

    /** @brief Advances falling/locking each tick; also drives the hard-drop instant-lock path. */
    void applyGravity();

    /**
     * @brief Adds score for cleared lines and updates level/speed.
     * @param [in] lines Number of lines cleared in this event.
     */
    void awardScoreAndLevel(int lines);

    /**
     * @brief Begins the blink animation for the given full rows.
     * @param [in] rows Row indices to blink before removal.
     */
    void startLineClearAnimation(const std::vector<int> &rows);

    /** @brief Advances the blink animation by one tick, finalizing the clear once it completes. */
    void updateLineClearAnimation();

    /** @brief Shows the game-over screen and blocks until the player restarts or quits. */
    void gameOver();
    /** @brief Refills the 7-bag piece queue with one shuffled instance of each shape. */
    void refillBag();

    /**
     * @brief Shows a yes/no confirmation overlay and blocks until answered.
     * @param [in] prompt Prompt text to display.
     * @retval true  The player confirmed (Y).
     * @retval false The player declined (N or Escape).
     */
    bool confirmAction(const std::string &prompt);

    /**
     * @brief Blocks (via short sleeps) until a key is available.
     * @return The next available key code.
     */
    int waitForKey();
};
