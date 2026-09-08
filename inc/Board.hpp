#pragma once
#include <array>
#include <vector>
#include "Tetromino.hpp"

// Owns the placed-block grid and all pure grid logic: collision, merging,
// line clearing, and ghost/current piece masks for rendering.
class Board
{
public:
    void reset();

    bool check(const Piece &p) const;
    void merge(const Piece &p);
    int clearLines();
    std::vector<int> findFullLines() const; // rows that are full, without removing them

    Piece getGhostPiece(const Piece &curr) const;
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> computeGhostMask(const Piece &curr) const;
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> computeCurrentMask(const Piece &curr) const;

    int cellAt(int row, int col) const { return field[row][col]; }

private:
    std::array<std::array<int, BOARD_WIDTH>, BOARD_HEIGHT> field{};
};
