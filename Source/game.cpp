//game.cpp
#include <iostream>
#include <string>
#include "Game.h"
#include "Score.h"

using namespace std;

const int MAX_PLAYERS = 8;

void playGame(int startingScore)
{
    string names[MAX_PLAYERS];
    int remScores[MAX_PLAYERS];
    int numPlayers = 0;

    
    while (numPlayers < 1 || numPlayers > MAX_PLAYERS)
    {
        cout << "How many players (1-8)? ";
        cin >> numPlayers;
    }


    for (int i = 0; i < numPlayers; i++)
    {
        cout << "Name of player " << i + 1 << ": ";
        cin >> names[i];
        remScores[i] = startingScore;
    }

    int current = 0;          // whose turn it is
    bool gameOver = false;

    while (!gameOver)
    {
        cout << "\n" << names[current] << "'s turn (needs " << remScores[current] << ")" << endl;

        int scoreBeforeVisit = remScores[current];   // so we can reset on a bust
        int visit = 0;
        DartResult result = CONTINUE;

        for (int i = 0; i < 3; i++)
        {
            int number;
            char multiplier;
            bool valid = false;

            cout << "\nDart " << i + 1 << endl;

            // Keep asking until the dart is valid
            while (!valid)
            {
                cout << "Enter number (0 = miss, 25 = bull): ";
                cin >> number;
                cout << "Enter multiplier (S, D, T): ";
                cin >> multiplier;

                valid = isValidDart(number, multiplier);

                if (!valid)
                    cout << "Invalid dart, try again." << endl;
            }

            int dartScore = calculateDartScore(number, multiplier);
            visit = visit + dartScore;

            result = applyDart(remScores[current], dartScore, multiplier);

            if (result != CONTINUE)   // won or bust, so stop throwing
                break;
        }

        if (result == WIN)
        {
            cout << "\n" << names[current] << " checks out and WINS!" << endl;
            gameOver = true;
        }
        else
        {
            if (result == BUST)
            {
                remScores[current] = scoreBeforeVisit;
                cout << "\nBUST! Score goes back to " << scoreBeforeVisit << endl;
            }
            else
            {
                cout << "\nVisit total: " << visit << "  Remaining: " << remScores[current] << endl;
            }

            // Next player, back to the first after the last one
            current++;
            if (current == numPlayers)
                current = 0;
        }
    }
}