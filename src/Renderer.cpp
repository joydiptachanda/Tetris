#include "Renderer.hpp"
#include <cstdio>
#include <cstdarg>
#include <unistd.h>

Renderer::Renderer() = default;

void Renderer::put(int row, int col, const std::string &text)
{
    term.moveCursor(row, col);
    term.write(text);
}

void Renderer::putf(int row, int col, const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    put(row, col, buf);
}

void Renderer::present()
{
    term.flush();
}

int Renderer::pollKey()
{
    return term.pollKey();
}

void Renderer::flushInput()
{
    term.flushInput();
}

void Renderer::drawGameBorder()
{
    std::string top = "┌";
    for (int i = 0; i < GAME_COLS - 2; ++i)
        top += "─";
    top += "┐";
    put(gameRow(0), gameCol(0), top);

    std::string bottom = "└";
    for (int i = 0; i < GAME_COLS - 2; ++i)
        bottom += "─";
    bottom += "┘";
    put(gameRow(GAME_ROWS - 1), gameCol(0), bottom);

    for (int y = 1; y < GAME_ROWS - 1; ++y)
    {
        put(gameRow(y), gameCol(0), "│");
        put(gameRow(y), gameCol(GAME_COLS - 1), "│");
    }
}

void Renderer::drawSideBorder()
{
    std::string horizontal = "+";
    for (int i = 0; i < SIDE_COLS - 2; ++i)
        horizontal += "=";
    horizontal += "+";
    put(sideRow(0), sideCol(0), horizontal);
    put(sideRow(SIDE_ROWS - 1), sideCol(0), horizontal);

    for (int y = 1; y < SIDE_ROWS - 1; ++y)
    {
        put(sideRow(y), sideCol(0), "|");
        put(sideRow(y), sideCol(SIDE_COLS - 1), "|");
    }
}

void Renderer::drawBoard(const Board &board, const Piece &curr)
{
    drawGameBorder();
    auto ghostMask = board.computeGhostMask(curr);
    auto currMask = board.computeCurrentMask(curr);
    drawCells(board, curr, ghostMask, currMask);
}

void Renderer::drawCells(const Board &board, const Piece &curr,
                         const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isGhostCell,
                         const std::array<std::array<bool, BOARD_WIDTH>, BOARD_HEIGHT> &isCurrCell)
{
    for (int i = 2; i < BOARD_HEIGHT; ++i)
    {
        for (int j = 0; j < BOARD_WIDTH; ++j)
        {
            int cell = board.cellAt(i, j);
            bool isCurrent = isCurrCell[i][j];
            bool isGhost = isGhostCell[i][j];
            int row = gameRow(i - 2 + 1);
            int col = gameCol(j * 2 + 1);

            if (isCurrent)
            {
                term.moveCursor(row, col);
                term.setColor(PIECE_ANSI_COLORS[curr.shape + 1], true, true);
                term.write("  ");
                term.resetAttrs();
            }
            else if (isGhost && !cell)
            {
                term.moveCursor(row, col);
                term.setColor(PIECE_ANSI_COLORS[curr.shape + 1], false, true, true);
                term.write("░░");
                term.resetAttrs();
            }
            else if (cell)
            {
                term.moveCursor(row, col);
                term.setColor(PIECE_ANSI_COLORS[cell], true);
                term.write("  ");
                term.resetAttrs();
            }
            else
            {
                put(row, col, "  ");
            }
        }
    }
}

void Renderer::drawInfo(int score, int level, const std::string &highscoreName, int highscoreScore,
                        const Piece &next, bool holding, const Piece &hold, bool paused)
{
    drawSideBorder();
    drawScorePanel(score, level, highscoreName, highscoreScore);
    drawNextPreview(next);
    drawHoldPreview(holding, hold);
    drawControls();
    drawPauseState(paused);
}

void Renderer::drawScorePanel(int score, int level, const std::string &highscoreName, int highscoreScore)
{
    putf(sideRow(1), sideCol(2), "Score: %d", score);
    putf(sideRow(2), sideCol(2), "Level: %d", level);
    putf(sideRow(3), sideCol(2), "Highscore: %s %d", highscoreName.c_str(), highscoreScore);
}

void Renderer::drawNextPreview(const Piece &next)
{
    put(sideRow(4), sideCol(2), "╔════ NEXT ════╗");
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            int row = sideRow(5 + y), col = sideCol(6 + x * 2);
            if (next.shape >= 0 && next.shape < 7 && TETROMINO_SHAPES[next.shape][0][y][x])
            {
                term.moveCursor(row, col);
                term.setColor(PIECE_ANSI_COLORS[next.shape + 1], true);
                term.write("  ");
                term.resetAttrs();
            }
            else
            {
                put(row, col, "  ");
            }
        }
    put(sideRow(9), sideCol(2), "╚══════════════╝");
}

void Renderer::drawHoldPreview(bool holding, const Piece &hold)
{
    put(sideRow(4), sideCol(19), "╔═══ HOLD ═══╗");
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            int row = sideRow(5 + y), col = sideCol(22 + x * 2);
            if (holding && hold.shape >= 0 && hold.shape < 7 && TETROMINO_SHAPES[hold.shape][0][y][x])
            {
                term.moveCursor(row, col);
                term.setColor(PIECE_ANSI_COLORS[hold.shape + 1], true);
                term.write("  ");
                term.resetAttrs();
            }
            else
            {
                put(row, col, "  ");
            }
        }
    put(sideRow(9), sideCol(19), "╚════════════╝");
}

void Renderer::drawControls()
{
    int row = 10;
    put(sideRow(row++), sideCol(2), "╔══════════ CONTROLS ═════════╗");
    put(sideRow(row++), sideCol(2), "← / →      : Move");
    put(sideRow(row++), sideCol(2), "↓          : Soft drop");
    put(sideRow(row++), sideCol(2), "Z/X        : Rotate");
    put(sideRow(row++), sideCol(2), "⎵/Space    : Hard drop");
    put(sideRow(row++), sideCol(2), "C          : Hold Piece");
    put(sideRow(row++), sideCol(2), "P          : Pause");
    put(sideRow(row++), sideCol(2), "H          : Clear Highscore");
    put(sideRow(row++), sideCol(2), "Q          : Quit");
    put(sideRow(row++), sideCol(2), "╚═════════════════════════════╝");
}

void Renderer::drawPauseState(bool paused)
{
    int row = 19; // Adjust if you expand controls
    if (paused)
        put(sideRow(row + 1), sideCol(2), "||══════════ PAUSED ═════════||");
}

void Renderer::clearOverlay()
{
    int y = BOARD_VISIBLE_HEIGHT / 2 + 1; // ansi row, matches the screens below
    int x = BOARD_WIDTH + 8 + 1;          // leftmost prompt column (highscore prompt is widest)
    std::string blank(50, ' ');
    for (int dy = -1; dy <= 3; ++dy)
        put(y + dy, x - 6, blank);
}

void Renderer::drawConfirmActionScreen(const std::string &prompt)
{
    int y = BOARD_VISIBLE_HEIGHT / 2 + 1, x = BOARD_WIDTH + 14 + 1;
    put(y - 1, x - 6, "+--------------------+");
    std::string msg = prompt.substr(0, 18);
    int pad = (18 - msg.length()) / 2;
    std::string line = "| ";
    line += std::string(pad, ' ');
    line += msg;
    line += std::string(18 - pad - msg.length(), ' ');
    line += " |";
    put(y, x - 6, line);
    put(y + 1, x - 6, "|    Y=YES   N=NO    |");
    put(y + 2, x - 6, "+--------------------+");
}

void Renderer::drawGameOverScreen()
{
    int y = BOARD_VISIBLE_HEIGHT / 2 + 1, x = BOARD_WIDTH + 14 + 1;
    put(y - 1, x - 6, "+--------------------+");
    put(y, x - 6, "|    GAME  OVER!     |");
    put(y + 1, x - 6, "| R=Restart  Q=Quit  |");
    put(y + 2, x - 6, "+--------------------+");
}

void Renderer::drawHighscorePrompt()
{
    int y = BOARD_VISIBLE_HEIGHT / 2 + 1, x = BOARD_WIDTH + 8 + 1;
    put(y - 1, x - 6, "+---------------------------------------+");
    put(y, x - 6, "| NEW HIGHSCORE! Enter name:            |");
    put(y + 1, x - 6, "+---------------------------------------+");
    nameInputRow = y;
    nameInputCol = x - 6 + 28; // 28 is after "NEW HIGHSCORE! Enter name: "
}

std::string Renderer::promptHighscoreName(size_t maxLen)
{
    std::string name;
    term.showCursor();
    term.moveCursor(nameInputRow, nameInputCol);
    present();

    while (true)
    {
        int key = term.pollKey();
        if (key == Terminal::None)
        {
            usleep(10 * 1000);
            continue;
        }
        if (key == '\n' || key == '\r')
            break;
        if (key == 127 || key == 8) // backspace
        {
            if (!name.empty())
            {
                name.pop_back();
                term.moveCursor(nameInputRow, nameInputCol + static_cast<int>(name.size()));
                term.write(" ");
                term.moveCursor(nameInputRow, nameInputCol + static_cast<int>(name.size()));
                present();
            }
            continue;
        }
        if (key >= 32 && key < 127 && name.size() < maxLen)
        {
            name += static_cast<char>(key);
            term.write(std::string(1, static_cast<char>(key)));
            present();
        }
    }

    term.hideCursor();
    present();
    return name;
}

