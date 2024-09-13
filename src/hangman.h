#ifndef HANGMAN_H
#define HANGMAN_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstring>
#include "ScreenBuffer.h"

using namespace std;

/// @brief Class representing the Hangman game.
class Hangman {
private:
    /// @brief ScreenBuffer object used to manage screen output.
    ScreenBuffer screenBuffer;

public:
    /// @brief Clears the screen using the ScreenBuffer class.
    void clearScreen();

    /// @brief Displays the hangman figure based on the number of wrong guesses.
    /// @param wrongGuesses The number of wrong guesses made by the player.
    void displayHangman(int wrongGuesses);

    /// @brief Retrieves a random word from a specified text file.
    /// @param filename The name of the file containing the words.
    /// @return A randomly selected word from the file.
    std::string getRandomWord(const std::string& filename);

    /// @brief Main gameplay loop for the Hangman game.
    /// @param difficulty The difficulty level selected by the player.
    void playHangman(const std::string& difficulty);

    /// @brief Checks if the response is a "yes".
    /// @param response The response string to check.
    /// @return True if the response is "yes", false otherwise.
    bool isYes(const std::string& response);

    /// @brief Checks if the response is a "no".
    /// @param response The response string to check.
    /// @return True if the response is "no", false otherwise.
    bool isNo(const std::string& response);

    /// @brief Runs the Hangman game.
    void hangman();
};

#endif // HANGMAN_H
