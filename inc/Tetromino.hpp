#pragma once
#include <array>

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 22;
constexpr int BOARD_VISIBLE_HEIGHT = 20;

struct Piece
{
    int shape, rot, x, y;
};

// Indexed as [shape][rotation][row][col]; 0 = empty, otherwise a piece_color_ids index.
extern const std::array<std::array<std::array<std::array<char, 4>, 4>, 4>, 7> TETROMINO_SHAPES;
extern const std::array<int, 8> PIECE_COLOR_IDS;
