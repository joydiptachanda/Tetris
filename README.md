# Tetris

A terminal Tetris clone written in C++11, with its own hand-rolled terminal
renderer (raw `termios` + ANSI escape codes) — no ncurses dependency.

## Features

- Classic 10x20 playfield with a 7-bag randomizer (each of the 7 pieces
  appears once per bag, so you never get unlucky droughts).
- Ghost piece preview showing where the current piece will land.
- Hold piece (swap the current piece out once per spawn).
- Next piece preview.
- Wall kicks on rotation (including the I-piece's wider kick table).
- Score, level, and speed progression as you clear lines.
- Blinking line-clear animation before cleared rows are removed.
- Persistent highscore (with name entry) saved to `highscore.txt`.
- Pause, restart, and quit, each with a confirmation prompt where relevant.

## Requirements

- A C++11 compiler (e.g. g++, clang++)
- CMake 3.10+
- A Linux terminal (the renderer uses raw `termios` + ANSI escapes)

No external libraries are required — the terminal rendering is entirely
self-contained (see [Architecture](#architecture) below).

## Build & Run

```sh
rm -rf build
mkdir build
cd build
cmake ..
make
./tetris
```

## Controls

| Key           | Action              |
|---------------|---------------------|
| `←` / `→`     | Move left / right   |
| `↓`           | Soft drop           |
| `Z` / `X`     | Rotate left / right |
| `Space`       | Hard drop           |
| `C`           | Hold piece          |
| `P`           | Pause               |
| `R`           | Restart             |
| `H`           | Clear highscore     |
| `Q`           | Quit                |

## Project structure

```
inc/            Public headers
src/            Implementation
  Tetromino.*   Shape/rotation/color data, shared Piece struct, board constants
  Board.*       Grid state, collision, line clearing/detection, ghost/current masks
  Renderer.*    All terminal drawing (board, side panel, prompts)
  Terminal.*    Raw termios + ANSI terminal I/O (no ncurses)
  HighscoreManager.* Highscore file persistence
  Logger.*      Simple timestamped file logger (tetris.log)
  TetrisGame.*  Game loop orchestration: input, gravity, scoring, spawning
  main.cpp      Entry point, signal handling
```

## Architecture

`TetrisGame` owns a `Board` (grid + collision rules), a `Renderer` (all
terminal drawing and input polling), and a `HighscoreManager` (persistence),
and coordinates them each tick: read input, apply gravity, redraw only what
changed. `Renderer` is the only place that talks to `Terminal`, which in turn
is the only code that touches raw terminal I/O — everything else is plain
C++ with no platform-specific dependencies.

## Static analysis

A `cppcheck`-based `analyze` CMake target is available if `cppcheck` is
installed:

```sh
cmake --build . --target analyze
```

## Logs & data files

- `tetris.log` — timestamped game log (gitignored)
- `highscore.txt` — persisted highscore name/score (gitignored)
