#include <csignal>
#include <ncurses.h>
#include <iostream>
#include "TetrisGame.hpp"

void handle_signal(int sig) {
    endwin();
    Logger::getInstance().log("\nTetris closed by signal " + sig);
    std::exit(1);
}

int main()
{
    std::signal(SIGINT, handle_signal);  // Ctrl-C
    std::signal(SIGTERM, handle_signal); // kill command
    TetrisGame game;
    game.run();
    return 0;
}
