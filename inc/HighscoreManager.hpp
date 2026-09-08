#pragma once
#include <string>

// Loads/saves the single-entry highscore file, independent of ncurses.
class HighscoreManager
{
public:
    void load();
    void save() const;
    void set(const std::string &newName, int newScore);

    const std::string &name() const { return highscore_name; }
    int score() const { return highscore_score; }

private:
    std::string highscore_name = "---";
    int highscore_score = 0;
};
