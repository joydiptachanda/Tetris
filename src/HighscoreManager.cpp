#include "HighscoreManager.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <vector>

void HighscoreManager::load()
{
    highscore_name = "---";
    highscore_score = 0;
    std::ifstream fin("highscore.txt");
    std::string line;
    if (std::getline(fin, line))
    {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok)
            tokens.push_back(tok);
        if (tokens.size() >= 2)
        {
            highscore_score = std::stoi(tokens.back());
            tokens.pop_back();
            highscore_name = "";
            for (size_t i = 0; i < tokens.size(); ++i)
            {
                if (i)
                    highscore_name += " ";
                highscore_name += tokens[i];
            }
        }
    }
    Logger::getInstance().log("Loaded highscore [" + highscore_name + "] " + std::to_string(highscore_score));
}

void HighscoreManager::save() const
{
    std::ofstream fout("highscore.txt");
    fout << highscore_name << " " << highscore_score << "\n";
    Logger::getInstance().log("Highscore: saved as [" + highscore_name + "] " + std::to_string(highscore_score));
}

void HighscoreManager::set(const std::string &newName, int newScore)
{
    highscore_name = newName;
    highscore_score = newScore;
}
