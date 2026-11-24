//game.cpp
#include <iostream>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>
#include "Game.h"
#include "Score.h"

using namespace std;

// Read an int, re-asking if the user types letters
static int readInt(const string& prompt)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value)
            return value;

        if (cin.eof())      // input closed, stop the program
            exit(0);

        cout << "Please enter a number." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void playGame(int startingScore)
{
    int numPlayers = readInt("How many players? ");
    while (numPlayers < 1)
        numPlayers = readInt("At least 1 player: ");

    // Later this can be swapped for Gary's Player objects
    vector<string> names(numPlayers);
    vector<int> remScores(numPlayers, startingScore);

    for (int i = 0; i < numPlayers; i++)
    {
        cout << "Name of player " << i + 1 << ": ";
        cin >> names[i];
    }

    int current = 0;

    while (true)
    {
        cout << "\n" << names[current] << "'s turn  (needs "
             << remScores[current] << ")" << endl;

        int scoreBeforeVisit = remScores[current];   // for resetting on a bust
        int visit = 0;
        DartResult result = CONTINUE;

        for (int i = 0; i < 3 && result == CONTINUE; i++)
        {
            int number;
            char multiplier;

            cout << "\nDart " << i + 1 << endl;

            // Keep asking until the dart is valid
            while (true)
            {
                number = readInt("Enter number (0 = miss, 25 = bull): ");
                cout << "Enter multiplier (S, D, T): ";
                cin >> multiplier;

                if (isValidDart(number, multiplier))
                    break;

                cout << "Invalid dart, try again." << endl;
            }

            int dartScore = calculateDartScore(number, multiplier);
            visit += dartScore;

            result = applyDart(remScores[current], dartScore, multiplier);
        }

        if (result == WIN)
        {
            cout << "\n" << names[current] << " checks out and WINS!" << endl;
            return;
        }

        if (result == BUST)
        {
            remScores[current] = scoreBeforeVisit;
            cout << "\nBUST! Score goes back to " << scoreBeforeVisit << endl;
        }
        else
        {
            cout << "\nVisit total: " << visit
                 << "  Remaining: " << remScores[current] << endl;
        }

        current = (current + 1) % numPlayers;   // next player
    }
}
