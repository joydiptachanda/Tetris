#include "TetrisGame.hpp"
#include <locale.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

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
      gameWin(nullptr), sideWin(nullptr)
{
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            field[i][j] = 0;

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

    // Precompute masks for ghost and current piece cells
    bool isGhostCell[HEIGHT][WIDTH] = {};
    bool isCurrCell[HEIGHT][WIDTH] = {};

    // Precompute ghost piece once
    const Piece ghost = getGhostPiece();
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (ghost.shape >= 0 && ghost.shape < 7 && ghost.rot >= 0 && ghost.rot < 4 &&
                tetromino[ghost.shape][ghost.rot][dy][dx])
            {
                int ny = ghost.y + dy, nx = ghost.x + dx;
                if (ny >= 2 && ny < HEIGHT && nx >= 0 && nx < WIDTH)
                    isGhostCell[ny][nx] = true;
            }

    // Precompute current piece
    for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx)
            if (curr.shape >= 0 && curr.shape < 7 && curr.rot >= 0 && curr.rot < 4 &&
                tetromino[curr.shape][curr.rot][dy][dx])
            {
                int ny = curr.y + dy, nx = curr.x + dx;
                if (ny >= 2 && ny < HEIGHT && nx >= 0 && nx < WIDTH)
                    isCurrCell[ny][nx] = true;
            }

    // Draw the board
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

    wnoutrefresh(gameWin);
}

void TetrisGame::drawInfo() const
{
    werase(sideWin);
    box(sideWin, 0, 0);

    mvwprintw(sideWin, 1, 2, "Score: %d", score);
    mvwprintw(sideWin, 2, 2, "Level: %d", level);

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

    if (paused)
    {
        int info_row = instructions_row + 1;
        wattron(sideWin, A_BOLD);
        mvwprintw(sideWin, info_row, 2, "-- PAUSED --");
        wattroff(sideWin, A_BOLD);
    }

    mvwprintw(sideWin, 3, 2, "Highscore: %s %d", highscore_name.c_str(), highscore_score);

    wnoutrefresh(sideWin);
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
    // First: handle pause toggle IMMEDIATELY
    if (ch == 'p' || ch == 'P')
    {
        paused = !paused;
        Logger::getInstance().log(paused ? "Game paused." : "Game resumed.");
        return; // Prevent other moves while toggling pause
    }

    // Implement "Hold Piece" Feature
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
        return;
    }

    // Implement "Clear highscore" Feature
    if ((ch == 'h' || ch == 'H'))
    {
        if (confirmAction("Clear highscore?"))
        {
            highscore_name = "---";
            highscore_score = 0;
            saveHighscore();
            Logger::getInstance().log("Highscore cleared by user.");
        }
        return;
    }

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
        return;
    }
    if ((ch == 'r' || ch == 'R') && !paused)
    {
        if (confirmAction("Restart game?"))
        {
            Logger::getInstance().log("Restart key pressed (confirmed).");
            // Game state reset (as in your restart handler)
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
            Logger::getInstance().log("Game restarted by user.");
        }
        else
        {
            Logger::getInstance().log("Restart cancelled by user.");
        }
        return;
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
        score += 1; // Soft drop bonus
        Logger::getInstance().log("Down key pressed. y=" + std::to_string(temp.y));
        break;
    case 'z':
    case 'x':
    {
        int old_rot = temp.rot;
        temp.rot = (ch == 'z') ? (temp.rot + 3) % 4 : (temp.rot + 1) % 4;

        bool kicked = false;
        if (curr.shape == 0)
        { // I piece
            const int kick_offsets[][2] = {
                {0, 0},
                {1, 0},
                {-1, 0},
                {-2, 0},
                {2, 0},
                {0, -1},
                {0, 1}};
            for (const auto &offset : kick_offsets)
            {
                Piece kicked_temp = temp;
                kicked_temp.x += offset[0];
                kicked_temp.y += offset[1];
                if (check(kicked_temp))
                {
                    temp = kicked_temp;
                    kicked = true;
                    break;
                }
            }
        }
        else
        { // Other pieces
            const int kick_offsets[][2] = {
                {0, 0},
                {1, 0},
                {-1, 0},
                {0, -1},
                {0, 1}};
            for (const auto &offset : kick_offsets)
            {
                Piece kicked_temp = temp;
                kicked_temp.x += offset[0];
                kicked_temp.y += offset[1];
                if (check(kicked_temp))
                {
                    temp = kicked_temp;
                    kicked = true;
                    break;
                }
            }
        }
        if (!kicked)
            temp.rot = old_rot;
        Logger::getInstance().log(
            std::string("Rotate ") + (ch == 'z' ? "left" : "right") +
            ", wall kick, rot=" + std::to_string(temp.rot));
        break;
    }
    case ' ':
    {
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
        Logger::getInstance().log("Hard drop to y=" + std::to_string(curr.y) +
                                  ", bonus: " + std::to_string(dropDistance * 2));
        break;
    }
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
                    break; // fallback
                }
                score += points;
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
        saveHighscore();
        Logger::getInstance().log(std::string("Name entered: [") + name_buf + "]");
        move(HEIGHT + 1, WIDTH * 2 + 5);
        clrtoeol();
        refresh();

        nodelay(stdscr, TRUE); // Set back to non-blocking for rest of game
    }

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
