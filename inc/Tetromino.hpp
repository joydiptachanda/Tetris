#pragma once
#include <array>

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 22;
constexpr int BOARD_VISIBLE_HEIGHT = 20;

struct Piece
{
    int shape, rot, x, y;
};

// Indexed as [shape][rotation][row][col]; 0 = empty, otherwise a piece_ansi_colors index.
extern const std::array<std::array<std::array<std::array<char, 4>, 4>, 4>, 7> TETROMINO_SHAPES;
// Standard ANSI SGR color numbers (0=black..7=white), indexed by piece shape+1.
extern const std::array<int, 8> PIECE_ANSI_COLORS;
