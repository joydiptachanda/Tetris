// =============================================================================
// Tetromino.hpp
// -----------------------------------------------------------------------------
// Shape/rotation/color data, the shared Piece struct, and board size
// constants used throughout the game. Pure data, no game state or I/O.
// =============================================================================
#pragma once
#include <array>

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 22;
constexpr int BOARD_VISIBLE_HEIGHT = 20;

/**
 * @brief A tetromino instance on the board.
 *
 * @details
 * Tracks which of the 7 shapes this piece is, its current rotation state,
 * and its top-left anchor position on the grid.
 */
struct Piece
{
    int shape, rot, x, y;
};

/**
 * @brief Tetromino rotation shape data.
 *
 * @details
 * Indexed as [shape][rotation][row][col]; 0 = empty, otherwise an index
 * into @ref PIECE_ANSI_COLORS.
 */
extern const std::array<std::array<std::array<std::array<char, 4>, 4>, 4>, 7> TETROMINO_SHAPES;

/**
 * @brief Per-piece display colors.
 *
 * @details
 * Standard ANSI SGR color numbers (0=black..7=white), indexed by piece shape+1.
 */
extern const std::array<int, 8> PIECE_ANSI_COLORS;
