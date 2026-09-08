#include "Renderer.hpp"
#include "Logger.hpp"
#include <algorithm>

Renderer::~Renderer()
{
    if (gameWin)
        delwin(gameWin);
    if (sideWin)
        delwin(sideWin);
}

void Renderer::init()
{
    gameWin = newwin(BOARD_VISIBLE_HEIGHT + 2, BOARD_WIDTH * 2 + 2, 1, 2);
    sideWin = newwin(BOARD_VISIBLE_HEIGHT + 2, 35, 1, BOARD_WIDTH * 2 + 4);
}

void Renderer::initColors()
{
    start_color();
    short bg = COLOR_BLACK;
#if NCURSES_VERSION_MAJOR >= 6
    if (use_default_colors() == OK)
        bg = -1;
#endif

    const std::array<short, 8> basic_color_ids = {{0,
                                                   COLOR_CYAN,
                                                   COLOR_YELLOW,
                                                   COLOR_MAGENTA,
                                                   COLOR_GREEN,
                                                   COLOR_RED,
                                                   COLOR_BLUE,
                                                   COLOR_WHITE}};

    const bool supports256 = (COLORS >= 256);
    const int safeColorCount = std::max(1, COLORS);
    for (int i = 1; i <= 7; ++i)
    {
        short fg = supports256 ? static_cast<short>(PIECE_COLOR_IDS[i]) : basic_color_ids[i];
        if (fg >= COLORS)
            fg = static_cast<short>(i % safeColorCount);

        if (init_pair(i, fg, bg) == ERR && bg == -1)
            init_pair(i, fg, COLOR_BLACK);
    }

    Logger::getInstance().log("Terminal COLORS=" + std::to_string(COLORS) +
                              ", using " + (supports256 ? std::string("256-color") : std::string("basic-color")) + " palette");
}

void Renderer::drawBoard(const Board &board, const Piece &curr)
{
    werase(gameWin);
    box(gameWin, 0, 0);
    auto ghostMask = board.computeGhostMask(curr);
    auto currMask = board.computeCurrentMask(curr);
    drawCells(board, curr, ghostMask, currMask);
    wnoutrefresh(gameWin);
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

            if (isCurrent)
            {
                wattron(gameWin, COLOR_PAIR(curr.shape + 1) | A_REVERSE | A_BOLD);
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "  ");
                wattroff(gameWin, COLOR_PAIR(curr.shape + 1) | A_REVERSE | A_BOLD);
            }
            else if (isGhost && !cell)
            {
                wattron(gameWin, COLOR_PAIR(curr.shape + 1) | A_DIM | A_BOLD);
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "░░");
                wattroff(gameWin, COLOR_PAIR(curr.shape + 1) | A_DIM | A_BOLD);
            }
            else if (cell)
            {
                wattron(gameWin, COLOR_PAIR(cell) | A_REVERSE);
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "  ");
                wattroff(gameWin, COLOR_PAIR(cell) | A_REVERSE);
            }
            else
            {
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "  ");
            }
        }
    }
}

void Renderer::drawInfo(int score, int level, const std::string &highscoreName, int highscoreScore,
                        const Piece &next, bool holding, const Piece &hold, bool paused)
{
    werase(sideWin);
    box(sideWin, 0, 0);
    wborder(sideWin, '|', '|', '=', '=', '+', '+', '+', '+');

    drawScorePanel(score, level, highscoreName, highscoreScore);
    drawNextPreview(next);
    drawHoldPreview(holding, hold);
    drawControls();
    drawPauseState(paused);

    wnoutrefresh(sideWin);
}

void Renderer::drawScorePanel(int score, int level, const std::string &highscoreName, int highscoreScore)
{
    mvwprintw(sideWin, 1, 2, "Score: %d", score);
    mvwprintw(sideWin, 2, 2, "Level: %d", level);
    mvwprintw(sideWin, 3, 2, "Highscore: %s %d", highscoreName.c_str(), highscoreScore);
}

void Renderer::drawNextPreview(const Piece &next)
{
    wattron(sideWin, A_BOLD | COLOR_PAIR(0));
    mvwprintw(sideWin, 4, 2, "╔════ NEXT ════╗");
    wattroff(sideWin, A_BOLD | COLOR_PAIR(next.shape + 1));
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            if (next.shape >= 0 && next.shape < 7 && TETROMINO_SHAPES[next.shape][0][y][x])
            {
                wattron(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
                mvwprintw(sideWin, 5 + y, 6 + x * 2, "  ");
                wattroff(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
            }
            else
            {
                mvwprintw(sideWin, 5 + y, 6 + x * 2, "  ");
            }
        }
    mvwprintw(sideWin, 9, 2, "╚══════════════╝");
}

void Renderer::drawHoldPreview(bool holding, const Piece &hold)
{
    wattron(sideWin, A_BOLD | COLOR_PAIR(0));
    mvwprintw(sideWin, 4, 19, "╔═══ HOLD ═══╗");
    wattroff(sideWin, A_BOLD | COLOR_PAIR(6));
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            if (holding && hold.shape >= 0 && hold.shape < 7 && TETROMINO_SHAPES[hold.shape][0][y][x])
            {
                wattron(sideWin, COLOR_PAIR(hold.shape + 1) | A_REVERSE);
                mvwprintw(sideWin, 5 + y, 22 + x * 2, "  ");
                wattroff(sideWin, COLOR_PAIR(hold.shape + 1) | A_REVERSE);
            }
            else
            {
                mvwprintw(sideWin, 5 + y, 22 + x * 2, "  ");
            }
        }
    mvwprintw(sideWin, 9, 19, "╚════════════╝");
}

void Renderer::drawControls()
{
    int instructions_row = 10;
    mvwprintw(sideWin, instructions_row++, 2, "╔══════════ CONTROLS ═════════╗");
    mvwprintw(sideWin, instructions_row++, 2, "← / →      : Move");
    mvwprintw(sideWin, instructions_row++, 2, "↓          : Soft drop");
    mvwprintw(sideWin, instructions_row++, 2, "Z/X        : Rotate");
    mvwprintw(sideWin, instructions_row++, 2, "⎵/Space    : Hard drop");
    mvwprintw(sideWin, instructions_row++, 2, "C          : Hold Piece");
    mvwprintw(sideWin, instructions_row++, 2, "P          : Pause");
    mvwprintw(sideWin, instructions_row++, 2, "H          : Clear Highscore");
    mvwprintw(sideWin, instructions_row++, 2, "Q          : Quit");
    mvwprintw(sideWin, instructions_row++, 2, "╚═════════════════════════════╝");
}

void Renderer::drawPauseState(bool paused)
{
    int instructions_row = 19; // Adjust if you expand controls
    if (paused)
    {
        wattron(sideWin, A_BOLD);
        mvwprintw(sideWin, instructions_row + 1, 2, "||══════════ PAUSED ═════════||");
        wattroff(sideWin, A_BOLD);
    }
}

void Renderer::drawConfirmActionScreen(const std::string &prompt)
{
    int y = BOARD_VISIBLE_HEIGHT / 2, x = BOARD_WIDTH + 14;
    wattron(stdscr, A_BOLD | COLOR_PAIR(0));
    mvprintw(y - 1, x - 6, "+--------------------+");
    std::string msg = prompt.substr(0, 18);
    int pad = (18 - msg.length()) / 2;
    std::string line = "| ";
    line += std::string(pad, ' ');
    line += msg;
    line += std::string(18 - pad - msg.length(), ' ');
    line += " |";
    mvprintw(y, x - 6, "%s", line.c_str());
    mvprintw(y + 1, x - 6, "|    Y=YES   N=NO    |");
    mvprintw(y + 2, x - 6, "+--------------------+");
    wattroff(stdscr, A_BOLD | COLOR_PAIR(0));
    refresh();
}

void Renderer::drawGameOverScreen()
{
    int y = BOARD_VISIBLE_HEIGHT / 2, x = BOARD_WIDTH + 14;
    wattron(stdscr, A_BOLD | COLOR_PAIR(0));
    mvprintw(y - 1, x - 6, "+--------------------+");
    mvprintw(y, x - 6, "|    GAME  OVER!     |");
    mvprintw(y + 1, x - 6, "| R=Restart  Q=Quit  |");
    mvprintw(y + 2, x - 6, "+--------------------+");
    wattroff(stdscr, A_BOLD | COLOR_PAIR(0));
    refresh();
}

void Renderer::drawHighscorePrompt()
{
    int y = BOARD_VISIBLE_HEIGHT / 2, x = BOARD_WIDTH + 8;
    wattron(stdscr, A_BOLD | COLOR_PAIR(0));
    mvprintw(y - 1, x - 6, "+---------------------------------------+");
    mvprintw(y, x - 6, "| NEW HIGHSCORE! Enter name:            |");
    mvprintw(y + 1, x - 6, "+---------------------------------------+");
    wattroff(stdscr, A_BOLD | COLOR_PAIR(0));
    refresh();
    move(y, x - 6 + 28); // 28 is after "NEW HIGHSCORE! Enter name: "
}
