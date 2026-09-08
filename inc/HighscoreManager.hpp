// =============================================================================
// HighscoreManager.hpp
// -----------------------------------------------------------------------------
// Loads and saves the single-entry highscore file (highscore.txt),
// independent of any rendering or input concerns.
// =============================================================================
#pragma once
#include <string>

/**
 * @brief Loads and saves the single-entry highscore file.
 *
 * @details
 * Persists to highscore.txt, independent of any rendering or input concerns.
 */
class HighscoreManager
{
public:
    /** @brief Loads the saved highscore from disk, defaulting to "---"/0 if none exists. */
    void load();

    /** @brief Persists the current highscore name/score to disk. */
    void save() const;

    /**
     * @brief Sets the in-memory highscore.
     * @param [in] newName  New highscore holder's name.
     * @param [in] newScore New highscore value.
     * @details Does not save to disk automatically; call save() separately.
     */
    void set(const std::string &newName, int newScore);

    /** @brief Returns the current highscore holder's name. */
    const std::string &name() const { return highscore_name; }
    /** @brief Returns the current highscore value. */
    int score() const { return highscore_score; }

private:
    std::string highscore_name = "---";
    int highscore_score = 0;
};
