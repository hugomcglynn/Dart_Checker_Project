//Menu.cpp
#include <iostream>
#include <limits>
#include "Menu.h"
#include "Game.h"

using namespace std;

static void newGameMenu()
{
    int choice = 0;

    cout << "\n1. Short game (301)" << endl;
    cout << "2. Long game (501)" << endl;
    cout << "3. Back" << endl;
    cout << "Enter choice: ";
    cin >> choice;

    if (choice == 1)
        playGame(301);
    else if (choice == 2)
        playGame(501);
}

void mainMenu()
{
    int choice = 0;

    while (choice != 8)   // keep showing the menu until Exit
    {
        cout << "\n================================" << endl;
        cout << "          DART CHECKER" << endl;
        cout << "================================" << endl;
        cout << "1. New Game" << endl;
        cout << "2. Game History" << endl;
        cout << "3. Leaderboards" << endl;
        cout << "4. Player Profiles" << endl;
        cout << "5. Clubs" << endl;
        cout << "6. Leagues" << endl;
        cout << "7. Tournaments" << endl;
        cout << "8. Exit" << endl;
        cout << "================================" << endl;
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }

        switch (choice)
        {
            case 1: newGameMenu(); break;
            case 2: cout << "Opening Game History..." << endl; break;   // Gary
            case 3: cout << "Opening Leaderboards..." << endl; break;   // Gary/Brendan
            case 4: cout << "Opening Player Profiles..." << endl; break; // Gary
            case 5: cout << "Opening Clubs..." << endl; break;          // Brendan
            case 6: cout << "Opening Leagues..." << endl; break;        // Brendan
            case 7: cout << "Opening Tournaments..." << endl; break;    // Brendan
            case 8: cout << "Exiting..." << endl; break;
            default: cout << "Invalid selection!" << endl;
        }
    }
}

