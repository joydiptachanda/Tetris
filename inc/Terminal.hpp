#pragma once
#include <string>
#include <termios.h>

// Minimal raw-mode terminal I/O built on termios + ANSI escape codes (Linux only).
// A from-scratch stand-in for the ncurses calls used by Renderer.
class Terminal
{
public:
    enum Key
    {
        None = -1,
        Escape = 27,
        Left = 1000,
        Right,
        Up,
        Down
    };

    Terminal();
    ~Terminal();

    Terminal(const Terminal &) = delete;
    Terminal &operator=(const Terminal &) = delete;

    void clear();
    void moveCursor(int row, int col); // 1-indexed, top-left origin
    void write(const std::string &s);
    void flush(); // sends the buffered frame in a single syscall

    void hideCursor();
    void showCursor();

    // ANSI SGR helpers; colorId 1-7 maps to the standard 8-color palette.
    void setColor(int colorId, bool reverse = false, bool bold = false, bool dim = false);
    void resetAttrs();

    // Non-blocking: returns Key::None if nothing is waiting.
    int pollKey();
    void flushInput(); // discards any currently buffered input

    // Best-effort terminal restore callable from a signal handler.
    static void emergencyRestore();

private:
    std::string frame;
    struct termios origTermios{};
    static Terminal *active;

    void enableRawMode();
    void disableRawMode();
};
