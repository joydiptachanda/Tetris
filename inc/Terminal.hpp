// =============================================================================
// Terminal.hpp
// -----------------------------------------------------------------------------
// Minimal raw-mode terminal I/O built on termios + ANSI escape codes
// (Linux only). A from-scratch stand-in for ncurses, used only by Renderer.
// =============================================================================
#pragma once
#include <string>
#include <termios.h>

/**
 * @brief Minimal raw-mode terminal I/O built on termios + ANSI escape codes.
 *
 * @details
 * Linux only. A from-scratch stand-in for ncurses, used only by Renderer.
 */
class Terminal
{
public:
    /** @brief Non-blocking key codes returned by pollKey(). */
    enum Key
    {
        None = -1,
        Escape = 27,
        Left = 1000,
        Right,
        Up,
        Down
    };

    /** @brief Enables raw mode and switches to the alternate screen buffer. */
    Terminal();
    /** @brief Restores the original terminal mode and screen buffer. */
    ~Terminal();

    // Owns live OS terminal state (raw mode, alternate screen buffer); copying
    // or moving would leave two objects trying to restore the same terminal.
    Terminal(const Terminal &) = delete;
    Terminal &operator=(const Terminal &) = delete;
    Terminal(Terminal &&) = delete;
    Terminal &operator=(Terminal &&) = delete;

    /** @brief Queues a full-screen clear. */
    void clear();

    /**
     * @brief Queues a cursor move.
     * @param [in] row Target row, 1-indexed with top-left origin.
     * @param [in] col Target column, 1-indexed with top-left origin.
     */
    void moveCursor(int row, int col);

    /**
     * @brief Queues raw text to be written.
     * @param [in] s Text to write at the current cursor position.
     */
    void write(const std::string &s);

    /** @brief Sends everything queued since the last flush() in a single syscall. */
    void flush();

    /** @brief Queues hiding the cursor. */
    void hideCursor();
    /** @brief Queues showing the cursor. */
    void showCursor();

    /**
     * @brief Queues an ANSI SGR color/attribute change.
     * @param [in] colorId Color number 1-7, mapping to the standard 8-color palette.
     * @param [in] reverse Swap foreground/background (used for solid filled blocks).
     * @param [in] bold    Apply the bold/bright attribute.
     * @param [in] dim     Apply the dim/faint attribute.
     */
    void setColor(int colorId, bool reverse = false, bool bold = false, bool dim = false);

    /** @brief Queues a reset of all color/attribute state to default. */
    void resetAttrs();

    /**
     * @brief Reads the next queued key.
     * @retval Key::None Nothing is waiting; returns immediately (non-blocking).
     * @retval other      The next available key code.
     */
    int pollKey();

    /** @brief Discards any currently buffered input. */
    void flushInput();

    /** @brief Best-effort terminal restore callable from a signal handler. */
    static void emergencyRestore();

private:
    std::string frame;
    struct termios origTermios{};
    static Terminal *active;

    /** @brief Puts stdin into raw, non-blocking mode. */
    void enableRawMode();
    /** @brief Restores stdin to its original mode. */
    void disableRawMode();
};
