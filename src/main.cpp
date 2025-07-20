#include <csignal>
#include <ncurses.h>
#include <iostream>
#include <string>
#include "TetrisGame.hpp"
#include "Logger.hpp"

// Graceful shutdown on Ctrl+C or kill
void handle_signal(int sig)
{
    endwin();
    Logger::getInstance().log("Tetris closed by signal " + std::to_string(sig));
    std::cerr << "\nTetris closed by signal " << sig << ".\n";
    std::exit(1);
}

// Terminal resize handler
void handle_winch(int sig)
{
    // Let ncurses know window size may have changed
    endwin();
    refresh();
    clear();
    // NOTE: The next game loop iteration will redraw everything.
    Logger::getInstance().log("Terminal resized (SIGWINCH received)");
}

int main()
{
    std::signal(SIGINT, handle_signal);  // Ctrl-C
    std::signal(SIGTERM, handle_signal); // kill command
    std::signal(SIGWINCH, handle_winch); // terminal resize

    TetrisGame game;
    game.run();
    return 0;
}
