#include <csignal>
#include <iostream>
#include <string>
#include "TetrisGame.hpp"
#include "Logger.hpp"
#include "Terminal.hpp"

// Graceful shutdown on Ctrl+C or kill
void handle_signal(int sig)
{
    Terminal::emergencyRestore();
    Logger::getInstance().log("Tetris closed by signal " + std::to_string(sig));
    std::cerr << "\nTetris closed by signal " << sig << ".\n";
    std::exit(1);
}

// Terminal resize handler
void handle_winch(int sig)
{
    (void)sig;
    // NOTE: The next game loop iteration redraws everything, so no
    // explicit refresh/clear is needed here.
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
