#include "TetrisGame.hpp"
#include <locale.h>
#include <unistd.h>
#include <cstdlib>

const char TetrisGame::tetromino[7][4][4][4] = {
    // I
    {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}},
     {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}},
    // O
    {{{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}},
    // T
    {{{0, 0, 0, 0}, {3, 3, 3, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}},
     {{0, 3, 0, 0}, {3, 3, 0, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}},
     {{0, 3, 0, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 3, 0, 0}, {0, 3, 3, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}}},
    // S
    {{{0, 0, 0, 0}, {0, 4, 4, 0}, {4, 4, 0, 0}, {0, 0, 0, 0}},
     {{4, 0, 0, 0}, {4, 4, 0, 0}, {0, 4, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {0, 4, 4, 0}, {4, 4, 0, 0}, {0, 0, 0, 0}},
     {{4, 0, 0, 0}, {4, 4, 0, 0}, {0, 4, 0, 0}, {0, 0, 0, 0}}},
    // Z
    {{{0, 0, 0, 0}, {5, 5, 0, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}},
     {{0, 5, 0, 0}, {5, 5, 0, 0}, {5, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {5, 5, 0, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}},
     {{0, 5, 0, 0}, {5, 5, 0, 0}, {5, 0, 0, 0}, {0, 0, 0, 0}}},
    // J
    {{{0, 6, 0, 0}, {0, 6, 0, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}},
     {{6, 0, 0, 0}, {6, 6, 6, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
     {{0, 6, 6, 0}, {0, 6, 0, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {6, 6, 6, 0}, {0, 0, 6, 0}, {0, 0, 0, 0}}},
    // L
    {{{0, 7, 0, 0}, {0, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}},
     {{0, 0, 0, 0}, {7, 7, 7, 0}, {7, 0, 0, 0}, {0, 0, 0, 0}},
     {{7, 7, 0, 0}, {0, 7, 0, 0}, {0, 7, 0, 0}, {0, 0, 0, 0}},
     {{0, 0, 7, 0}, {7, 7, 7, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}};

const int TetrisGame::piece_color_ids[8] = {
    0,   // unused
    51,  // I - Bright Cyan
    226, // O - Yellow
    201, // T - Magenta
    46,  // S - Green
    196, // Z - Red
    21,  // J - Blue
    214  // L - Orange
};

TetrisGame::TetrisGame()
    : score(0), level(1), delay(500), frame(0), running(true),
      gameWin(nullptr), sideWin(nullptr)
{
    setlocale(LC_ALL, "");
    srand((unsigned)time(0));
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            field[i][j] = 0;
    initscr();
    if (has_colors())
        initColors256();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    gameWin = newwin(VISIBLE_HEIGHT + 2, WIDTH * 2 + 2, 1, 2);
    sideWin = newwin(VISIBLE_HEIGHT + 2, 32, 1, WIDTH * 2 + 4);

    spawnPiece();
    next = {rand() % 7, 0, WIDTH / 2 - 2, 0};
    paused = false;
    Logger::getInstance().log("Game initialized.");
}

TetrisGame::~TetrisGame()
{
    endwin();
}

void TetrisGame::initColors256()
{
    start_color();
#if NCURSES_VERSION_MAJOR >= 6
    use_default_colors();
#endif
    for (int i = 1; i <= 7; ++i)
        init_pair(i, piece_color_ids[i], -1);
}

bool TetrisGame::check(const Piece &p) const
{
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (tetromino[p.shape][p.rot][dy][dx])
            {
                int ny = p.y + dy, nx = p.x + dx;
                if (ny < 0 || ny >= HEIGHT || nx < 0 || nx >= WIDTH)
                    return false;
                if (field[ny][nx])
                    return false;
            }
    return true;
}

void TetrisGame::merge(const Piece &p)
{
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (tetromino[p.shape][p.rot][dy][dx])
            {
                int ny = p.y + dy, nx = p.x + dx;
                field[ny][nx] = p.shape + 1;
            }
}

int TetrisGame::clearLines()
{
    int lines = 0;
    for (int i = HEIGHT - 1; i >= 0; i--)
    {
        bool full = true;
        for (int j = 0; j < WIDTH; ++j)
            if (!field[i][j])
                full = false;
        if (full)
        {
            ++lines;
            for (int k = i; k > 0; --k)
                for (int j = 0; j < WIDTH; ++j)
                    field[k][j] = field[k - 1][j];
            for (int j = 0; j < WIDTH; ++j)
                field[0][j] = 0;
            ++i;
        }
    }
    return lines;
}

// Function to Find Ghost Position
TetrisGame::Piece TetrisGame::getGhostPiece() const
{
    Piece ghost = curr;
    while (check(ghost))
        ++ghost.y;
    --ghost.y;
    return ghost;
}

void TetrisGame::drawBoard() const
{
    werase(gameWin);
    box(gameWin, 0, 0);

    // Draw the board, ghost, and current piece
    Piece ghost = getGhostPiece();

    for (int i = 2; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            int cell = field[i][j];
            bool isGhost = false, isCurrent = false;

            // Is this cell occupied by the ghost piece?
            for (int dy = 0; dy < 4; ++dy)
                for (int dx = 0; dx < 4; ++dx)
                    if (tetromino[ghost.shape][ghost.rot][dy][dx] &&
                        ghost.y + dy == i && ghost.x + dx == j)
                        isGhost = true;

            // Is this cell occupied by the current piece?
            for (int dy = 0; dy < 4; ++dy)
                for (int dx = 0; dx < 4; ++dx)
                    if (tetromino[curr.shape][curr.rot][dy][dx] &&
                        curr.y + dy == i && curr.x + dx == j)
                        isCurrent = true;

            // Draw logic
            if (isCurrent)
            {
                wattron(gameWin, COLOR_PAIR(curr.shape + 1) | A_REVERSE);
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "  ");
                wattroff(gameWin, COLOR_PAIR(curr.shape + 1) | A_REVERSE);
            }
            else if (isGhost && !cell)
            {
                wattron(gameWin, COLOR_PAIR(curr.shape + 1) | A_DIM);
                mvwprintw(gameWin, i - 2 + 1, j * 2 + 1, "::");
                wattroff(gameWin, COLOR_PAIR(curr.shape + 1) | A_DIM);
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

    wnoutrefresh(gameWin);
}

void TetrisGame::drawInfo() const
{
    werase(sideWin);
    box(sideWin, 0, 0);

    // Score and Level
    mvwprintw(sideWin, 1, 2, "Score: %d", score);
    mvwprintw(sideWin, 2, 2, "Level: %d", level);

    // Next piece label
    mvwprintw(sideWin, 4, 2, "Next:");

    // Next piece preview
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            if (tetromino[next.shape][0][y][x])
            {
                wattron(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
                mvwprintw(sideWin, 6 + y, x * 2 + 2, "  ");
                wattroff(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
            }
            else
            {
                mvwprintw(sideWin, 6 + y, x * 2 + 2, "  ");
            }

    // Instructions (controls)
    int instructions_row = 11;
    mvwprintw(sideWin, instructions_row++, 2, "Controls:");
    mvwprintw(sideWin, instructions_row++, 2, "Left/Right : Move");
    mvwprintw(sideWin, instructions_row++, 2, "Down       : Soft drop");
    mvwprintw(sideWin, instructions_row++, 2, "Z/X        : Rotate");
    mvwprintw(sideWin, instructions_row++, 2, "Space      : Hard drop");
    mvwprintw(sideWin, instructions_row++, 2, "P          : Pause");
    mvwprintw(sideWin, instructions_row++, 2, "Q          : Quit");

    // pause functionality
    if (paused)
    {
        int info_row = 18;
        wattron(sideWin, A_BOLD);
        mvwprintw(sideWin, info_row, 2, "-- PAUSED --");
        wattroff(sideWin, A_BOLD);
    }

    wnoutrefresh(sideWin);
}

void TetrisGame::spawnPiece()
{
    curr = next;
    next = {rand() % 7, 0, WIDTH / 2 - 2, 0};
    Logger::getInstance().log("Spawning piece: " + std::to_string(curr.shape) +
                              " at x=" + std::to_string(curr.x) + ", y=" + std::to_string(curr.y));
}

void TetrisGame::handleInput(int ch)
{
    // First: handle pause toggle IMMEDIATELY
    if (ch == 'p' || ch == 'P')
    {
        paused = !paused;
        Logger::getInstance().log(paused ? "Game paused." : "Game resumed.");
        return; // Prevent other moves while toggling pause
    }

    // If paused, ignore ALL other inputs
    if (paused)
        return;

    Piece temp = curr;
    switch (ch)
    {
    case KEY_LEFT:
        --temp.x;
        Logger::getInstance().log("Left key pressed. x=" + std::to_string(temp.x));
        break;
    case KEY_RIGHT:
        ++temp.x;
        Logger::getInstance().log("Right key pressed. x=" + std::to_string(temp.x));
        break;
    case KEY_DOWN:
        ++temp.y;
        Logger::getInstance().log("Down key pressed. y=" + std::to_string(temp.y));
        break;
    case 'z':
        temp.rot = (temp.rot + 3) % 4;
        Logger::getInstance().log("Rotate left. rot=" + std::to_string(temp.rot));
        break;
    case 'x':
        temp.rot = (temp.rot + 1) % 4;
        Logger::getInstance().log("Rotate right. rot=" + std::to_string(temp.rot));
        break;
    case ' ':
        Logger::getInstance().log("Hard drop.");
        while (check(temp))
            ++temp.y;
        --temp.y;
        curr = temp;
        Logger::getInstance().log("Hard drop to y=" + std::to_string(curr.y));
        break;
    case 'q':
        Logger::getInstance().log("Quit key pressed.");
        running = false;
        return; // added instead of break to no longer see log entries after "Quit key pressed."
    }
    if (check(temp) && ch != ' ')
    {
        curr = temp;
        Logger::getInstance().log("Piece moved to (x=" +
                                  std::to_string(curr.x) + ", y=" + std::to_string(curr.y) +
                                  ", rot=" + std::to_string(curr.rot) + ")");
    }
}

void TetrisGame::applyGravity(int ch)
{
    if (++frame > delay / 30 || ch == KEY_DOWN)
    {
        frame = 0;
        Piece fall = curr;
        ++fall.y;
        if (check(fall))
        {
            curr = fall;
            Logger::getInstance().log("Piece falls to y=" + std::to_string(curr.y));
        }
        else
        {
            Logger::getInstance().log("Piece cannot fall; merging at (x=" +
                                      std::to_string(curr.x) + ", y=" + std::to_string(curr.y) + ")");
            merge(curr);
            int lines = clearLines();
            if (lines > 0)
            {
                score += lines * 100;
                Logger::getInstance().log("Cleared lines: " + std::to_string(lines));
                Logger::getInstance().log("Score: " + std::to_string(score));
                level = score / 500 + 1;
                delay = std::max(100, 500 - (level - 1) * 40);
                Logger::getInstance().log("Level: " + std::to_string(level) +
                                          ", Delay: " + std::to_string(delay));
            }
            spawnPiece();
            if (!check(curr))
            {
                Logger::getInstance().log("Game Over: spawn not possible.");
                gameOver();
            }
        }
    }
}

void TetrisGame::gameOver()
{
    mvprintw(HEIGHT, WIDTH * 2 + 5, "GAME OVER! Press Q...");
    nodelay(stdscr, FALSE);
    while (getch() != 'q')
        ;
    running = false;
}

void TetrisGame::run()
{
    while (running)
    {
        usleep(30 * 1000);
        int ch = getch();
        handleInput(ch);
        if (!paused)
        {
            applyGravity(ch);
        }
        drawBoard();
        drawInfo();
        doupdate();
    }
}
