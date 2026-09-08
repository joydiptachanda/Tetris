#include "Board.hpp"
#include <algorithm>

void Board::reset()
{
    field = {};
}

bool Board::check(const Piece &p) const
{
    if (p.shape < 0 || p.shape >= 7 || p.rot < 0 || p.rot >= 4)
        return false;
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (TETROMINO_SHAPES[p.shape][p.rot][dy][dx])
            {
                int ny = p.y + dy, nx = p.x + dx;
                if (ny < 0 || ny >= BOARD_HEIGHT || nx < 0 || nx >= BOARD_WIDTH)
                    return false;
                if (field[ny][nx])
                    return false;
            }
    return true;
}

void Board::merge(const Piece &p)
{
    if (p.shape < 0 || p.shape >= 7 || p.rot < 0 || p.rot >= 4)
        return;
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (TETROMINO_SHAPES[p.shape][p.rot][dy][dx])
            {
                int ny = p.y + dy, nx = p.x + dx;
                field[ny][nx] = p.shape + 1;
            }
}

int Board::clearLines()
{
    int lines = 0;
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--)
    {
        if (std::all_of(field[i].begin(), field[i].end(), [](int x)
                        { return x != 0; }))
        {
            ++lines;
            for (int k = i; k > 0; --k)
                field[k] = field[k - 1];
            field[0] = {};
            ++i;
        }
    }
    return lines;
}

std::vector<int> Board::findFullLines() const
{
    std::vector<int> rows;
    for (int i = 0; i < BOARD_HEIGHT; ++i)
        if (std::all_of(field[i].begin(), field[i].end(), [](int x)
                        { return x != 0; }))
            rows.push_back(i);
    return rows;
}

Piece Board::getGhostPiece(const Piece &curr) const
{
    Piece ghost = curr;
    while (check(ghost))
        ++ghost.y;
    --ghost.y;
    return ghost;
}

std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> Board::computeGhostMask(const Piece &curr) const
{
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> ghostMask{};
    const Piece ghost = getGhostPiece(curr);
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (ghost.shape >= 0 && ghost.shape < 7 && ghost.rot >= 0 && ghost.rot < 4 &&
                TETROMINO_SHAPES[ghost.shape][ghost.rot][dy][dx])
            {
                int ny = ghost.y + dy, nx = ghost.x + dx;
                if (ny >= 2 && ny < BOARD_HEIGHT && nx >= 0 && nx < BOARD_WIDTH)
                    ghostMask[ny][nx] = true;
            }
    return ghostMask;
}

std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> Board::computeCurrentMask(const Piece &curr) const
{
    std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> currMask{};
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (curr.shape >= 0 && curr.shape < 7 && curr.rot >= 0 && curr.rot < 4 &&
                TETROMINO_SHAPES[curr.shape][curr.rot][dy][dx])
            {
                int ny = curr.y + dy, nx = curr.x + dx;
                if (ny >= 2 && ny < BOARD_HEIGHT && nx >= 0 && nx < BOARD_WIDTH)
                    currMask[ny][nx] = true;
            }
    return currMask;
}
