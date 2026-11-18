// Menu.cpp
#include "menu.h"
#include "Competitions.h"
#include "Game.h"
#include "MatchHistory.h"
#include "PlayerRecords.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>

using namespace std;

namespace
{
int menuNumber(const string& prompt, int low, int high)
{
    string line;
    while (cout << prompt && getline(cin, line))
    {
        istringstream input(line);
        int number;
        char extra;
        if (input >> number && !(input >> extra) && number >= low && number <= high)
            return number;
        cout << "Invalid selection!\n";
    }
    return 0;
}

void newGameMenu(PlayerRecords& players, MatchHistory& history)
{
    cout << "\n1. Short game (301)\n2. Long game (501)\n3. Back\n";
    int choice = menuNumber("Enter choice: ", 1, 3);
    if (choice == 0 || choice == 3)
        return;

    int count = menuNumber("How many players (2-8, 0 back)? ", 0, 8);
    if (count == 0)
        return;
    if (count == 1)
    {
        cout << "At least two players are needed.\n";
        return;
    }

    int ids[8];
    string names[8];
    for (int i = 0; i < count; ++i)
    {
        cout << "\nChoose player " << i + 1 << ".\n";
        ids[i] = players.selectPlayer(true);
        if (ids[i] <= 0)
            return;
        bool alreadyPicked = false;
        for (int j = 0; j < i; ++j)
            if (ids[j] == ids[i])
                alreadyPicked = true;
        if (alreadyPicked)
        {
            cout << "That player is already in this game.\n";
            --i;
            continue;
        }
        names[i] = players.name(ids[i]);
    }

    MatchResult result{};
    if (playMatch(choice == 1 ? 301 : 501, ids, names, count, result)
        && history.record(result, players))
        cout << "Match saved.\n";
}
}

int mainMenu()
{
    PlayerRecords players("dart_players.txt");
    MatchHistory history("dart_history.txt");
    const string competitionFile = "dart_competitions.txt";
    if (!players.load() || !history.load())
    {
        cerr << "Could not load the player or match save. Check its location and contents.\n";
        return 1;
    }

    int choice = 0;
    while (choice != 8)
    {
        cout << "\n================================\n";
        cout << "          DART CHECKER\n";
        cout << "================================\n";
        cout << "1. New Game\n";
        cout << "2. Game History\n";
        cout << "3. Leaderboards\n";
        cout << "4. Player Profiles\n";
        cout << "5. Clubs\n";
        cout << "6. Leagues\n";
        cout << "7. Tournaments\n";
        cout << "8. Exit\n";
        cout << "================================\n";

        choice = menuNumber("Enter choice: ", 1, 8);
        switch (choice)
        {
            case 1: newGameMenu(players, history); break;
            case 2: history.show(); break;
            case 3: history.showLeaderboard(players, competitionFile); break;
            case 4: players.accountMenu(history); break;
            case 5:
            case 6:
            case 7: competitionMenu(choice, players, history, players.isAdmin(), competitionFile); break;
            case 8: cout << "Exiting...\n"; break;
            case 0: return 0;
        }
    }
    return 0;
}

int demo()
{
    error_code error;
    filesystem::path directory = filesystem::temp_directory_path(error);
    if (error)
    {
        cerr << "Could not find a folder for demo data.\n";
        return 1;
    }
    auto timestamp = chrono::system_clock::now().time_since_epoch().count();
    directory /= "dart-checker-demo-" + to_string(timestamp);
    if (!filesystem::create_directory(directory, error))
    {
        cerr << "Could not create the demo folder.\n";
        return 1;
    }

    PlayerRecords players((directory / "dart_players.txt").string());
    MatchHistory history((directory / "dart_history.txt").string());
    string competitionFile = (directory / "dart_competitions.txt").string();
    const string names[4] = {"Alex Byrne", "Casey Doyle", "Jamie Kelly", "Riley Walsh"};
    int ids[4];
    cout << "\nDart Checker Demo\nDemo data folder: " << directory.string() << '\n';
    for (int i = 0; i < 4; ++i)
    {
        ids[i] = players.addPlayer(names[i], 20 + i, "Irish", "Fictional demo address", "DartsDemoOnly");
        if (ids[i] <= 0)
            return 1;
    }

    MatchResult result{};
    if (!playMatch(301, ids, names, 2, result, true) || !history.record(result, players)
        || !playMatch(501, ids + 2, names + 2, 2, result, true) || !history.record(result, players)
        || !runCompetitionDemo(players, history, competitionFile, ids))
    {
        cerr << "Could not complete the demo.\n";
        return 1;
    }

    history.show();
    history.showLeaderboard(players, competitionFile);
    return 0;
}
