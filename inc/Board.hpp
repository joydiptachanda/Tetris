// =============================================================================
// Board.hpp
// -----------------------------------------------------------------------------
// Owns the placed-block grid and all pure grid logic: collision, merging,
// line clearing, and ghost/current piece masks for rendering.
// =============================================================================
#pragma once
#include <array>
#include <vector>
#include "Tetromino.hpp"

/**
 * @brief Owns the placed-block grid and all pure grid logic.
 *
 * @details
 * Handles collision, merging, line clearing, and ghost/current piece masks
 * for rendering. Has no dependency on rendering or input.
 */
class Board
{
public:
    /** @brief Clears the grid back to empty. */
    void reset();

    /**
     * @brief Checks whether a piece fits on the grid.
     * @param [in] p Piece to check.
     * @retval true  p fits without colliding or going out of bounds.
     * @retval false p collides with a locked cell or the grid boundary.
     */
    bool check(const Piece &p) const;

    /**
     * @brief Locks a piece into the grid.
     * @param [in] p Piece whose occupied cells are written into the grid as locked blocks.
     */
    void merge(const Piece &p);

    /**
     * @brief Removes all full rows and shifts the rows above them down.
     * @return The number of lines cleared.
     */
    int clearLines();

    /**
     * @brief Detects full rows without mutating the grid.
     * @return The indices of all full rows.
     */
    std::vector<int> findFullLines() const;

    /**
     * @brief Computes the hard-drop landing position of a piece.
     * @param [in] curr Piece to project downward.
     * @return curr dropped straight down until it would collide.
     */
    Piece getGhostPiece(const Piece &curr) const;

    /**
     * @brief Computes which cells the ghost piece occupies.
     * @param [in] curr Piece whose ghost projection is computed.
     * @return A per-cell mask of where curr's ghost piece occupies the grid.
     */
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> computeGhostMask(const Piece &curr) const;

    /**
     * @brief Computes which cells the current piece occupies.
     * @param [in] curr Piece whose occupied cells are computed.
     * @return A per-cell mask of where curr currently occupies the grid.
     */
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> computeCurrentMask(const Piece &curr) const;

    /**
     * @brief Reads a single cell's value.
     * @param [in] row Row index.
     * @param [in] col Column index.
     * @return 0 if empty, otherwise a @ref PIECE_ANSI_COLORS index.
     */
    int cellAt(int row, int col) const { return field[row][col]; }

private:
    std::array<std::array<int, BOARD_WIDTH>, BOARD_HEIGHT> field{};
};
