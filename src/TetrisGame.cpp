#include "TetrisGame.hpp"
#include <locale.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

const std::array<std::array<std::array<std::array<char, 4>, 4>, 4>, 7> TetrisGame::tetromino = {{

    // I
    {{{{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}}},
      {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}}}},

    // O
    {{{{{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}}}}},

    // T
    {{{{{0, 0, 0, 0}, {3, 3, 3, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 3, 0, 0}, {3, 3, 0, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 3, 0, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 3, 0, 0}, {0, 3, 3, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}}}}},

    // S
    {{{{{0, 0, 0, 0}, {0, 4, 4, 0}, {4, 4, 0, 0}, {0, 0, 0, 0}}},
      {{{4, 0, 0, 0}, {4, 4, 0, 0}, {0, 4, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {0, 4, 4, 0}, {4, 4, 0, 0}, {0, 0, 0, 0}}},
      {{{4, 0, 0, 0}, {4, 4, 0, 0}, {0, 4, 0, 0}, {0, 0, 0, 0}}}}},

    // Z
    {{{{{0, 0, 0, 0}, {5, 5, 0, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}}},
      {{{0, 5, 0, 0}, {5, 5, 0, 0}, {5, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {5, 5, 0, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}}},
      {{{0, 5, 0, 0}, {5, 5, 0, 0}, {5, 0, 0, 0}, {0, 0, 0, 0}}}}},

    // J
    {{{{{0, 6, 0, 0}, {0, 6, 0, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}}},
      {{{6, 0, 0, 0}, {6, 6, 6, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 6, 6, 0}, {0, 6, 0, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {6, 6, 6, 0}, {0, 0, 6, 0}, {0, 0, 0, 0}}}}},

    // L
    {{{{{0, 7, 0, 0}, {0, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 0, 0}, {7, 7, 7, 0}, {7, 0, 0, 0}, {0, 0, 0, 0}}},
      {{{7, 7, 0, 0}, {0, 7, 0, 0}, {0, 7, 0, 0}, {0, 0, 0, 0}}},
      {{{0, 0, 7, 0}, {7, 7, 7, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}}}

}};

const std::array<int, 8> TetrisGame::piece_color_ids = {{
    0,   // unused
    51,  // I - Bright Cyan
    226, // O - Yellow
    201, // T - Magenta
    46,  // S - Green
    196, // Z - Red
    21,  // J - Blue
    214  // L - Orange
}};

// For improved randomization
void TetrisGame::refillBag()
{
    std::vector<int> bag = {0, 1, 2, 3, 4, 5, 6};
    static std::random_device rd;
    static std::mt19937 g(rd());
    std::shuffle(bag.begin(), bag.end(), g);
    for (int i : bag)
        pieceQueue.push(i);
}

TetrisGame::TetrisGame()
    : score(0), level(1), delay(500), frame(0), running(true),
      gameWin(nullptr), sideWin(nullptr),
      hardDropped(false), boardDirty(true), infoDirty(true)
{
    field = {};

    setlocale(LC_ALL, "");
    srand((unsigned)time(0));

    initscr();
    if (has_colors())
        initColors256();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    gameWin = newwin(VISIBLE_HEIGHT + 2, WIDTH * 2 + 2, 1, 2);
    sideWin = newwin(VISIBLE_HEIGHT + 2, 35, 1, WIDTH * 2 + 4);

    loadHighscore();

    // Implement "Hold Piece" Feature
    holding = false;          // No held piece at start
    holdUsedThisTurn = false; // Can hold at the beginning

    refillBag();
    if (pieceQueue.empty())
        refillBag();
    int pieceType = pieceQueue.front();
    pieceQueue.pop();
    next = {pieceType, 0, WIDTH / 2 - 2, 0};

    paused = false;
    boardDirty = infoDirty = true;
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
    if (p.shape < 0 || p.shape >= 7 || p.rot < 0 || p.rot >= 4)
        return false;
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
    if (p.shape < 0 || p.shape >= 7 || p.rot < 0 || p.rot >= 4)
        return;
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
    auto ghostMasks = computeGhostMask();
    auto currMasks = computeCurrentMask();
    drawCells(ghostMasks, currMasks);
    wnoutrefresh(gameWin);
}

std::array<std::array<bool, TetrisGame::WIDTH>, TetrisGame::HEIGHT>
TetrisGame::computeGhostMask() const
{
    std::array<std::array<bool, WIDTH>, HEIGHT> ghostMask{};
    const Piece ghost = getGhostPiece();
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (ghost.shape >= 0 && ghost.shape < 7 && ghost.rot >= 0 && ghost.rot < 4 &&
                tetromino[ghost.shape][ghost.rot][dy][dx])
            {
                int ny = ghost.y + dy, nx = ghost.x + dx;
                if (ny >= 2 && ny < HEIGHT && nx >= 0 && nx < WIDTH)
                    ghostMask[ny][nx] = true;
            }
    return ghostMask;
}

std::array<std::array<bool, TetrisGame::WIDTH>, TetrisGame::HEIGHT>
TetrisGame::computeCurrentMask() const
{
    std::array<std::array<bool, WIDTH>, HEIGHT> currMask{};
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (curr.shape >= 0 && curr.shape < 7 && curr.rot >= 0 && curr.rot < 4 &&
                tetromino[curr.shape][curr.rot][dy][dx])
            {
                int ny = curr.y + dy, nx = curr.x + dx;
                if (ny >= 2 && ny < HEIGHT && nx >= 0 && nx < WIDTH)
                    currMask[ny][nx] = true;
            }
    return currMask;
}

void TetrisGame::drawCells(
    const std::array<std::array<bool, WIDTH>, HEIGHT> &isGhostCell,
    const std::array<std::array<bool, WIDTH>, HEIGHT> &isCurrCell) const
{
    for (int i = 2; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            int cell = field[i][j];
            bool isCurrent = isCurrCell[i][j];
            bool isGhost = isGhostCell[i][j];

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
}

void TetrisGame::drawInfo() const
{
    werase(sideWin);
    box(sideWin, 0, 0);

    drawScorePanel();
    drawNextPreview();
    drawHoldPreview();
    drawControls();
    drawPauseState();

    wnoutrefresh(sideWin);
}

void TetrisGame::drawScorePanel() const
{
    mvwprintw(sideWin, 1, 2, "Score: %d", score);
    mvwprintw(sideWin, 2, 2, "Level: %d", level);
    mvwprintw(sideWin, 3, 2, "Highscore: %s %d", highscore_name.c_str(), highscore_score);
}

void TetrisGame::drawNextPreview() const
{
    mvwprintw(sideWin, 4, 2, "Next:");
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            if (next.shape >= 0 && next.shape < 7 && tetromino[next.shape][0][y][x])
            {
                wattron(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
                mvwprintw(sideWin, 6 + y, 2 + x * 2, "  ");
                wattroff(sideWin, COLOR_PAIR(next.shape + 1) | A_REVERSE);
            }
            else
            {
                mvwprintw(sideWin, 6 + y, 2 + x * 2, "  ");
            }
        }
}

void TetrisGame::drawHoldPreview() const
{
    mvwprintw(sideWin, 4, 14, "Hold:");
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
        {
            if (holding && hold.shape >= 0 && hold.shape < 7 && tetromino[hold.shape][0][y][x])
            {
                wattron(sideWin, COLOR_PAIR(hold.shape + 1) | A_REVERSE);
                mvwprintw(sideWin, 6 + y, 14 + x * 2, "  ");
                wattroff(sideWin, COLOR_PAIR(hold.shape + 1) | A_REVERSE);
            }
            else
            {
                mvwprintw(sideWin, 6 + y, 14 + x * 2, "  ");
            }
        }
}

void TetrisGame::drawControls() const
{
    int instructions_row = 10;
    mvwprintw(sideWin, instructions_row++, 2, "Controls:");
    mvwprintw(sideWin, instructions_row++, 2, "=============================");
    mvwprintw(sideWin, instructions_row++, 2, "Left/Right : Move");
    mvwprintw(sideWin, instructions_row++, 2, "Down       : Soft drop");
    mvwprintw(sideWin, instructions_row++, 2, "Z/X        : Rotate");
    mvwprintw(sideWin, instructions_row++, 2, "Space      : Hard drop");
    mvwprintw(sideWin, instructions_row++, 2, "C          : Hold Piece");
    mvwprintw(sideWin, instructions_row++, 2, "P          : Pause");
    mvwprintw(sideWin, instructions_row++, 2, "H          : Clear Highscore");
    mvwprintw(sideWin, instructions_row++, 2, "Q          : Quit");
}

void TetrisGame::drawPauseState() const
{
    int instructions_row = 19; // Adjust if you expand controls
    if (paused)
    {
        wattron(sideWin, A_BOLD);
        mvwprintw(sideWin, instructions_row + 1, 2, "-- PAUSED --");
        wattroff(sideWin, A_BOLD);
    }
}

void TetrisGame::spawnPiece()
{
    curr = next;
    if (pieceQueue.empty())
        refillBag();
    int pieceType = pieceQueue.front();
    pieceQueue.pop();
    next = {pieceType, 0, WIDTH / 2 - 2, 0};

    // Implement "Hold Piece" Feature
    holdUsedThisTurn = false;
    Logger::getInstance().log("Spawning piece: " + std::to_string(curr.shape) +
                              " at x=" + std::to_string(curr.x) + ", y=" + std::to_string(curr.y));
}

void TetrisGame::handleInput(int ch)
{
    bool didMove = false;
    bool didRotate = false;
    bool didDrop = false;

    if (handlePauseKey(ch))
    {
        infoDirty = true;
        return;
    }
    if (handleHoldKey(ch))
    {
        infoDirty = true;
        return;
    }
    if (handleClearHighscoreKey(ch))
    {
        infoDirty = true;
        return;
    }
    if (handleQuitKey(ch))
        return;
    if (handleRestartKey(ch))
    {
        boardDirty = infoDirty = true;
        return;
    }

    if (paused)
        return;

    Piece temp = curr;

    if (handleMoveKey(ch, temp))
    {
        curr = temp;
        boardDirty = true;
        return;
    }
    if (handleRotateKey(ch, temp))
    {
        curr = temp;
        boardDirty = true;
        return;
    }
    if (handleDropKey(ch, temp))
    {
        curr = temp;
        boardDirty = true;
        return;
    }
}

bool TetrisGame::handlePauseKey(int ch)
{
    if (ch == 'p' || ch == 'P')
    {
        paused = !paused;
        infoDirty = true;
        Logger::getInstance().log(paused ? "Game paused." : "Game resumed.");
        return true;
    }
    return false;
}

bool TetrisGame::handleHoldKey(int ch)
{
    if ((ch == 'c' || ch == 'C') && !holdUsedThisTurn)
    {
        Logger::getInstance().log("Hold key pressed.");
        if (!holding)
        {
            hold = curr;
            holding = true;
            spawnPiece();
        }
        else
        {
            std::swap(curr, hold);
            curr.x = WIDTH / 2 - 2;
            curr.y = 0;
            curr.rot = 0;
        }
        holdUsedThisTurn = true;
        infoDirty = true;
        return true;
    }
    return false;
}

bool TetrisGame::handleClearHighscoreKey(int ch)
{
    if ((ch == 'h' || ch == 'H'))
    {
        if (confirmAction("Clear highscore?"))
        {
            highscore_name = "---";
            highscore_score = 0;
            saveHighscore();
            Logger::getInstance().log("Highscore cleared by user.");
            infoDirty = true;
        }
        return true;
    }
    return false;
}

bool TetrisGame::handleQuitKey(int ch)
{
    if ((ch == 'q' || ch == 'Q') && !paused)
    {
        if (confirmAction("Quit game?"))
        {
            Logger::getInstance().log("Quit key pressed (confirmed).");
            running = false;
        }
        else
        {
            Logger::getInstance().log("Quit cancelled by user.");
        }
        return true;
    }
    return false;
}

bool TetrisGame::handleRestartKey(int ch)
{
    if ((ch == 'r' || ch == 'R') && !paused)
    {
        if (confirmAction("Restart game?"))
        {
            Logger::getInstance().log("Restart key pressed (confirmed).");
            field = {};
            score = 0;
            level = 1;
            delay = 500;
            frame = 0;
            holding = false;
            holdUsedThisTurn = false;
            paused = false;
            pieceQueue = std::queue<int>();
            refillBag();
            if (pieceQueue.empty())
                refillBag();
            int pieceType = pieceQueue.front();
            pieceQueue.pop();
            next = {pieceType, 0, WIDTH / 2 - 2, 0};
            spawnPiece();
            boardDirty = infoDirty = true;
            Logger::getInstance().log("Game restarted by user.");
        }
        else
        {
            Logger::getInstance().log("Restart cancelled by user.");
        }
        return true;
    }
    return false;
}

bool TetrisGame::handleMoveKey(int ch, Piece &temp)
{
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
        score += 1;
        infoDirty = true;
        Logger::getInstance().log("Down key pressed. y=" + std::to_string(temp.y));
        break;
    default:
        return false;
    }
    if (check(temp))
    {
        Logger::getInstance().log("Piece moved to (x=" + std::to_string(temp.x) +
                                  ", y=" + std::to_string(temp.y) +
                                  ", rot=" + std::to_string(temp.rot) + ")");
        return true;
    }
    return false;
}

bool TetrisGame::handleRotateKey(int ch, Piece &temp)
{
    if (ch != 'z' && ch != 'x')
        return false;
    int old_rot = temp.rot;
    temp.rot = (ch == 'z') ? (temp.rot + 3) % 4 : (temp.rot + 1) % 4;
    bool kicked = false;
    if (curr.shape == 0)
    { // I piece
        const int kick_offsets[][2] = {{0, 0}, {1, 0}, {-1, 0}, {-2, 0}, {2, 0}, {0, -1}, {0, 1}};
        for (const auto &o : kick_offsets)
        {
            Piece ktemp = temp;
            ktemp.x += o[0];
            ktemp.y += o[1];
            if (check(ktemp))
            {
                temp = ktemp;
                kicked = true;
                break;
            }
        }
    }
    else
    {
        const int kick_offsets[][2] = {{0, 0}, {1, 0}, {-1, 0}, {0, -1}, {0, 1}};
        for (const auto &o : kick_offsets)
        {
            Piece ktemp = temp;
            ktemp.x += o[0];
            ktemp.y += o[1];
            if (check(ktemp))
            {
                temp = ktemp;
                kicked = true;
                break;
            }
        }
    }
    if (!kicked)
        temp.rot = old_rot;
    Logger::getInstance().log(std::string("Rotate ") + (ch == 'z' ? "left" : "right") +
                              ", wall kick, rot=" + std::to_string(temp.rot));
    if (check(temp))
        return true;
    return false;
}

bool TetrisGame::handleDropKey(int ch, Piece &temp)
{
    if (ch != ' ')
        return false;
    int dropDistance = 0;
    while (check(temp))
    {
        ++temp.y;
        ++dropDistance;
    }
    --temp.y;
    --dropDistance;
    curr = temp;
    score += dropDistance * 2;
    infoDirty = true;
    boardDirty = true;
    Logger::getInstance().log("Hard drop to y=" + std::to_string(curr.y) +
                              ", bonus: " + std::to_string(dropDistance * 2));
    hardDropped = true;
    return true;
}

void TetrisGame::applyGravity(int ch)
{
    // Instant lock if hard drop was performed
    if (hardDropped)
    {
        Logger::getInstance().log("Piece instantly merged from hard drop.");
        merge(curr);
        boardDirty = true;
        int lines = clearLines();
        awardScoreAndLevel(lines);
        spawnPiece();
        infoDirty = true; // Next/hold panel may change
        if (!check(curr))
        {
            Logger::getInstance().log("Game Over: spawn not possible.");
            infoDirty = true;
            boardDirty = true;
            gameOver();
        }
        hardDropped = false;
        return;
    }

    // Normal gravity handling
    if (++frame > delay / 30 || ch == KEY_DOWN)
    {
        frame = 0;
        Piece fall = curr;
        ++fall.y;
        if (check(fall))
        {
            curr = fall;
            boardDirty = true;
            Logger::getInstance().log("Piece falls to y=" + std::to_string(curr.y));
        }
        else
        {
            Logger::getInstance().log("Piece cannot fall; merging at (x=" +
                                      std::to_string(curr.x) + ", y=" + std::to_string(curr.y) + ")");
            merge(curr);
            boardDirty = true;
            int lines = clearLines();
            awardScoreAndLevel(lines);
            spawnPiece();
            infoDirty = true;
            if (!check(curr))
            {
                Logger::getInstance().log("Game Over: spawn not possible.");
                infoDirty = boardDirty = true;
                gameOver();
            }
        }
    }
}

void TetrisGame::awardScoreAndLevel(int lines)
{
    if (lines <= 0)
        return;

    int points = 0;
    switch (lines)
    {
    case 1:
        points = 100 * level;
        break;
    case 2:
        points = 300 * level;
        break;
    case 3:
        points = 500 * level;
        break;
    case 4:
        points = 800 * level;
        break;
    default:
        points = lines * 100 * level;
        break;
    }
    score += points;
    Logger::getInstance().log("Cleared lines: " + std::to_string(lines));
    Logger::getInstance().log("Score: " + std::to_string(score));
    level = score / 500 + 1;
    delay = std::max(100, 500 - (level - 1) * 40);
    Logger::getInstance().log("Level: " + std::to_string(level) + ", Delay: " + std::to_string(delay));
    infoDirty = true;
}

void TetrisGame::saveHighscore()
{
    std::ofstream fout("highscore.txt");
    fout << highscore_name << " " << highscore_score << "\n";
    Logger::getInstance().log("Highscore: saved as [" + highscore_name + "] " + std::to_string(highscore_score));
}

void TetrisGame::loadHighscore()
{
    highscore_name = "---";
    highscore_score = 0;
    std::ifstream fin("highscore.txt");
    std::string line;
    if (std::getline(fin, line))
    {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok)
            tokens.push_back(tok);
        if (tokens.size() >= 2)
        {
            highscore_score = std::stoi(tokens.back());
            tokens.pop_back();
            highscore_name = "";
            for (size_t i = 0; i < tokens.size(); ++i)
            {
                if (i)
                    highscore_name += " ";
                highscore_name += tokens[i];
            }
        }
    }
    Logger::getInstance().log("Loaded highscore [" + highscore_name + "] " + std::to_string(highscore_score));
}

bool TetrisGame::confirmAction(const std::string &prompt)
{
    // Choose a location for the prompt
    int y = HEIGHT / 2, x = (WIDTH * 2) / 2;
    mvprintw(y, x, "%s (Y/N):   ", prompt.c_str());
    refresh();
    int response;
    bool result = false;
    while (true)
    {
        response = getch();
        if (response == 'y' || response == 'Y')
        {
            result = true;
            break;
        }
        if (response == 'n' || response == 'N' || response == 27 /* ESC */)
        {
            result = false;
            break;
        }
    }
    // Clean up the prompt line
    move(y, x);
    clrtoeol();
    refresh();
    return result;
}

void TetrisGame::gameOver()
{
    // CHECK AND UPDATE HIGHSCORE FIRST!
    if (score > highscore_score)
    {
        // BLOCKING MODE and FLUSH buffered KEYS
        nodelay(stdscr, FALSE);
        flushinp();

        char name_buf[32] = "---";
        move(HEIGHT + 1, WIDTH * 2 + 5);
        clrtoeol();
        mvprintw(HEIGHT + 1, WIDTH * 2 + 5, "NEW HIGHSCORE! Enter name: ");
        echo();
        curs_set(1);
        getnstr(name_buf, 31);
        curs_set(0);
        noecho();

        if (name_buf[0] == '\0')
            strcpy(name_buf, "---");
        highscore_name = name_buf;
        highscore_score = score;
        infoDirty = true;
        saveHighscore();
        Logger::getInstance().log(std::string("Name entered: [") + name_buf + "]");
        move(HEIGHT + 1, WIDTH * 2 + 5);
        clrtoeol();
        refresh();

        nodelay(stdscr, TRUE); // Set back to non-blocking for rest of game
    }
    infoDirty = true;
    boardDirty = true;

    mvprintw(HEIGHT, WIDTH * 2 + 5, "GAME OVER! Press R=Restart, Q=Quit...");
    nodelay(stdscr, FALSE);

    int k;
    while (true)
    {
        k = getch();
        if (k == 'r' || k == 'R')
        {
            // Reset all fields
            for (int i = 0; i < HEIGHT; ++i)
                for (int j = 0; j < WIDTH; ++j)
                    field[i][j] = 0;
            score = 0;
            level = 1;
            delay = 500;
            frame = 0;
            holding = false;
            holdUsedThisTurn = false;
            paused = false;
            pieceQueue = std::queue<int>();
            refillBag();
            if (pieceQueue.empty())
                refillBag();
            int pieceType = pieceQueue.front();
            pieceQueue.pop();
            next = {pieceType, 0, WIDTH / 2 - 2, 0};
            spawnPiece();
            running = true;
            nodelay(stdscr, TRUE);
            // Clear the game over message line after restart or quit
            move(HEIGHT, WIDTH * 2 + 5);
            clrtoeol(); // Clear to end of line, requires #include <ncurses.h>
            refresh();
            Logger::getInstance().log("Game restarted.");
            return;
        }
        else if (k == 'q' || k == 'Q')
        {
            // Clear the game over message line after restart or quit
            move(HEIGHT, WIDTH * 2 + 5);
            clrtoeol(); // Clear to end of line, requires #include <ncurses.h>
            refresh();
            running = false;
            break;
        }
    }
}

void TetrisGame::run()
{
    boardDirty = true;
    infoDirty = true;
    while (running)
    {
        usleep(30 * 1000);
        int ch = getch();

        handleInput(ch);
        if (!paused)
            applyGravity(ch);

        if (boardDirty)
            drawBoard();
        if (infoDirty)
            drawInfo();
        doupdate();

        boardDirty = false;
        infoDirty = false;
    }
}
