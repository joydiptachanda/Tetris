#include "TetrisGame.hpp"
#include <locale.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <random>

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
      hardDropped(false), boardDirty(true), infoDirty(true)
{
    board.reset();

    setlocale(LC_ALL, "");
    srand((unsigned)time(0));

    initscr();
    if (has_colors())
        renderer.initColors();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    renderer.init();

    highscore.load();

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

void TetrisGame::restartGame()
{
    board.reset();
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
}

void TetrisGame::redrawAll()
{
    renderer.drawBoard(board, curr);
    renderer.drawInfo(score, level, highscore.name(), highscore.score(), next, holding, hold, paused);
    doupdate();
}

void TetrisGame::handleInput(int ch)
{
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
        if (confirmAction("CLEAR HIGHSCORE?"))
        {
            highscore.set("---", 0);
            highscore.save();
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
        if (confirmAction("QUIT GAME?"))
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
        if (confirmAction("RESTART GAME?"))
        {
            Logger::getInstance().log("Restart key pressed (confirmed).");
            restartGame();
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
    bool softDropAttempt = false;

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
        softDropAttempt = true;
        ++temp.y;
        Logger::getInstance().log("Down key pressed. y=" + std::to_string(temp.y));
        break;
    default:
        return false;
    }
    if (board.check(temp))
    {
        if (softDropAttempt)
        {
            score += 1;
            infoDirty = true;
        }
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
            if (board.check(ktemp))
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
            if (board.check(ktemp))
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
    if (board.check(temp))
        return true;
    return false;
}

bool TetrisGame::handleDropKey(int ch, Piece &temp)
{
    if (ch != ' ')
        return false;
    int dropDistance = 0;
    while (board.check(temp))
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

void TetrisGame::applyGravity()
{
    // Instant lock if hard drop was performed
    if (hardDropped)
    {
        Logger::getInstance().log("Piece instantly merged from hard drop.");
        board.merge(curr);
        boardDirty = true;
        int lines = board.clearLines();
        awardScoreAndLevel(lines);
        spawnPiece();
        infoDirty = true; // Next/hold panel may change
        if (!board.check(curr))
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
    if (++frame > delay / 30)
    {
        frame = 0;
        Piece fall = curr;
        ++fall.y;
        if (board.check(fall))
        {
            curr = fall;
            boardDirty = true;
            Logger::getInstance().log("Piece falls to y=" + std::to_string(curr.y));
        }
        else
        {
            Logger::getInstance().log("Piece cannot fall; merging at (x=" +
                                      std::to_string(curr.x) + ", y=" + std::to_string(curr.y) + ")");
            board.merge(curr);
            boardDirty = true;
            int lines = board.clearLines();
            awardScoreAndLevel(lines);
            spawnPiece();
            infoDirty = true;
            if (!board.check(curr))
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

bool TetrisGame::confirmAction(const std::string &prompt)
{
    int y = VISIBLE_HEIGHT / 2, x = WIDTH + 14;
    renderer.drawConfirmActionScreen(prompt);
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
        if (response == 'n' || response == 'N' || response == 27)
        {
            result = false;
            break;
        }
    }

    for (int dy = -1; dy <= 2; ++dy)
    {
        move(y + dy, x - 6);
        clrtoeol();
    }
    refresh();

    redrawAll();

    return result;
}

void TetrisGame::gameOver()
{
    // CHECK AND UPDATE HIGHSCORE FIRST!
    if (score > highscore.score())
    {
        // BLOCKING MODE and FLUSH buffered KEYS
        nodelay(stdscr, FALSE);
        flushinp();

        char name_buf[32] = "---";
        move(HEIGHT + 1, WIDTH * 2 + 5);
        clrtoeol();
        renderer.drawHighscorePrompt();
        echo();
        curs_set(1);
        getnstr(name_buf, 31);
        curs_set(0);
        noecho();

        if (name_buf[0] == '\0')
            strcpy(name_buf, "---");
        highscore.set(name_buf, score);
        infoDirty = true;
        highscore.save();
        Logger::getInstance().log(std::string("Name entered: [") + name_buf + "]");
        move(HEIGHT + 1, WIDTH * 2 + 5);
        clrtoeol();
        refresh();

        nodelay(stdscr, TRUE); // Set back to non-blocking for rest of game
    }
    infoDirty = true;
    boardDirty = true;

    renderer.drawGameOverScreen();
    nodelay(stdscr, FALSE);

    int k;
    while (true)
    {
        k = getch();
        if (k == 'r' || k == 'R')
        {
            restartGame();
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
            applyGravity();

        if (boardDirty)
            renderer.drawBoard(board, curr);
        if (infoDirty)
            renderer.drawInfo(score, level, highscore.name(), highscore.score(), next, holding, hold, paused);
        doupdate();

        boardDirty = false;
        infoDirty = false;
    }
}
