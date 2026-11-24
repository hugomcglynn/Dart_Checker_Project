// Original menu by Hugo McGlynn. Competition menu changes by Brendan Galvin.
// App integration by Codex.

#include "menu.h"
#include "Competitions.h"
#include "Game.h"
#include "MatchHistory.h"
#include "PlayerRecords.h"
#include <chrono>
#include <filesystem>
#include <iostream>

using namespace std;

namespace
{
void newGameMenu(PlayerRecords& players, MatchHistory& history)
{
    cout << "\n1. Short game (301)\n2. Long game (501)\n3. Back\n";
    int choice = askNumber("Enter choice: ", 1, 3, false);
    if (choice == 0 || choice == 3)
        return;

    int count = askNumber("How many players (2-8, 0 back)? ", 2, MAX_MATCH_PLAYERS);
    if (count == 0)
        return;

    int ids[MAX_MATCH_PLAYERS];
    string names[MAX_MATCH_PLAYERS];
    int picked = 0;
    while (picked < count)
    {
        cout << "\nChoose player " << picked + 1 << ".\n";
        int id = players.selectPlayer(true);
        if (id <= 0)
            return;
        bool alreadyPicked = false;
        for (int i = 0; i < picked; ++i)
            if (ids[i] == id)
                alreadyPicked = true;
        if (alreadyPicked)
        {
            cout << "That player is already in this game.\n";
            continue;
        }
        ids[picked] = id;
        names[picked] = players.name(id);
        ++picked;
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

    while (true)
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

        int choice = askNumber("Enter choice: ", 1, 8, false);
        switch (choice)
        {
            case 1: newGameMenu(players, history); break;
            case 2: history.show(); break;
            case 3: history.showLeaderboard(players, competitionFile); break;
            case 4: players.accountMenu(history); break;
            case 5:
            case 6:
            case 7: competitionMenu(choice, players, history, players.isAdmin(), competitionFile); break;
            case 8: cout << "Exiting...\n"; return 0;
            case 0: return 0;
        }
    }
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

    const int scores[] = {301, 501};
    for (int i = 0; i < 2; ++i)
    {
        MatchResult result{};
        if (!playMatch(scores[i], ids + 2 * i, names + 2 * i, 2, result, true)
            || !history.record(result, players))
        {
            cerr << "Could not save the demo match.\n";
            return 1;
        }
    }
    if (!runCompetitionDemo(players, history, competitionFile, ids))
    {
        cerr << "Could not finish the demo competitions.\n";
        return 1;
    }

    history.show();
    history.showLeaderboard(players, competitionFile);
    return 0;
}
