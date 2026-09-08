#include "Terminal.hpp"
#include <unistd.h>
#include <fcntl.h>

Terminal *Terminal::active = nullptr;

Terminal::Terminal()
{
    enableRawMode();
    write("\x1b[?1049h"); // switch to the alternate screen buffer
    hideCursor();
    flush();
    active = this;
}

Terminal::~Terminal()
{
    if (active == this)
        active = nullptr;
    showCursor();
    write("\x1b[?1049l"); // restore the primary screen buffer
    flush();
    disableRawMode();
}

void Terminal::enableRawMode()
{
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void Terminal::disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

void Terminal::clear()
{
    frame += "\x1b[2J";
}

void Terminal::moveCursor(int row, int col)
{
    frame += "\x1b[" + std::to_string(row) + ";" + std::to_string(col) + "H";
}

void Terminal::write(const std::string &s)
{
    frame += s;
}

void Terminal::flush()
{
    if (!frame.empty())
    {
        ::write(STDOUT_FILENO, frame.data(), frame.size());
        frame.clear();
    }
}

void Terminal::hideCursor()
{
    frame += "\x1b[?25l";
}

void Terminal::showCursor()
{
    frame += "\x1b[?25h";
}

void Terminal::setColor(int colorId, bool reverse, bool bold, bool dim)
{
    frame += "\x1b[0"; // reset first so attributes never stack across calls
    if (bold)
        frame += ";1";
    if (dim)
        frame += ";2";
    if (reverse)
        frame += ";7";
    if (colorId > 0)
        frame += ";3" + std::to_string(colorId % 8);
    frame += "m";
}

void Terminal::resetAttrs()
{
    frame += "\x1b[0m";
}

void Terminal::flushInput()
{
    unsigned char discard;
    while (::read(STDIN_FILENO, &discard, 1) > 0)
    {
    }
}

void Terminal::emergencyRestore()
{
    if (!active)
        return;
    static const char kRestore[] = "\x1b[?25h\x1b[?1049l";
    ::write(STDOUT_FILENO, kRestore, sizeof(kRestore) - 1);
    active->disableRawMode();
}

int Terminal::pollKey()
{
    unsigned char c;
    if (::read(STDIN_FILENO, &c, 1) <= 0)
        return None;

    if (c != 0x1b)
        return c;

    // Escape sequence: arrow keys are ESC [ A/B/C/D.
    unsigned char seq[2];
    if (::read(STDIN_FILENO, &seq[0], 1) <= 0)
        return Escape;
    if (::read(STDIN_FILENO, &seq[1], 1) <= 0)
        return Escape;

    if (seq[0] == '[')
    {
        switch (seq[1])
        {
        case 'A':
            return Up;
        case 'B':
            return Down;
        case 'C':
            return Right;
        case 'D':
            return Left;
        }
    }
    return Escape;
}
