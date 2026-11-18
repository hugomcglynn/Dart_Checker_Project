#include "Competitions.h"
#include "Club.h"
#include "Game.h"
#include "League.h"
#include "MatchHistory.h"
#include "PlayerRecords.h"
#include "Tournament.h"
#include <algorithm>
#include <climits>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

Competition::Competition(string competitionName, const vector<int>& entrants)
{
    name = competitionName;
    players = entrants;
}

string Competition::getName() const { return name; }
const vector<int>& Competition::entrants() const { return players; }
const vector<Fixture>& Competition::matches() const { return fixtures; }

bool Competition::recordResult(int matchNumber, int winnerId)
{
    if (matchNumber < 1 || matchNumber > static_cast<int>(fixtures.size()))
        return false;

    Fixture& match = fixtures[matchNumber - 1];
    if (match.winner != 0 || match.first == 0 || match.second == 0)
        return false;
    if (winnerId != match.first && winnerId != match.second)
        return false;

    match.winner = winnerId;
    return true;
}

bool Competition::recordLegacyResult(int matchNumber, int winnerId)
{
    if (!recordResult(matchNumber, winnerId))
        return false;
    fixtures[matchNumber - 1].legacyWinner = winnerId;
    return true;
}

namespace
{
struct Data
{
    vector<Club> clubs;
    vector<League> leagues;
    vector<Tournament> cups;
};

void writeCompetition(ostream& out, char type, const Competition& event, int prize = 0)
{
    out << type << ' ' << quoted(event.getName()) << ' ' << event.entrants().size();
    for (int playerId : event.entrants())
        out << ' ' << playerId;
    // Keep older winner-only results. New results live in match history.
    for (const Fixture& match : event.matches())
        out << ' ' << match.legacyWinner;
    if (type == 'T')
        out << ' ' << prize;
    out << '\n';
}

bool saveData(const Data& data, const string& file)
{
    string temporary = file + ".tmp";
    ofstream out(temporary);
    for (const Club& club : data.clubs)
        club.save(out);
    for (const League& league : data.leagues)
        writeCompetition(out, 'L', league);
    for (const Tournament& cup : data.cups)
        writeCompetition(out, 'T', cup, cup.getPrize());

    out.close();
    if (!out)
        return false;

    // Replace the previous save only after the new file has been written.
    error_code error;
    filesystem::rename(temporary, file, error);
    return !error;
}

bool readResults(istream& in, Competition& event)
{
    for (int i = 1; i <= static_cast<int>(event.matches().size()); i++)
    {
        int winner;
        if (!(in >> winner) || (winner != 0 && !event.recordLegacyResult(i, winner)))
            return false;
    }
    return true;
}

bool loadData(Data& data, const string& file)
{
    ifstream in(file);
    if (!in)
    {
        error_code error;
        bool exists = filesystem::exists(file, error);
        return !error && !exists;
    }

    string line;
    while (getline(in, line))
    {
        if (line.find_first_not_of(" \t\r") == string::npos)
            continue;
        istringstream row(line);
        char type;
        string name;
        int count;
        if (!(row >> type >> quoted(name) >> count) || name.empty() ||
            count < 0 || count > MAX_CLUB_PLAYERS)
            return false;

        vector<int> ids;
        for (int i = 0; i < count; i++)
        {
            int id;
            if (!(row >> id) || id < 1 || find(ids.begin(), ids.end(), id) != ids.end())
                return false;
            ids.push_back(id);
        }

        if (type == 'C')
        {
            Club club(name);
            for (int id : ids)
                club.addPlayer(id);
            data.clubs.push_back(club);
        }
        else if (type == 'L' && count >= 2 && count <= MAX_LEAGUE_PLAYERS)
        {
            League league(name, ids);
            if (!readResults(row, league))
                return false;
            data.leagues.push_back(league);
        }
        else if (type == 'T' && count == 4)
        {
            int winners[3];
            if (!(row >> winners[0] >> winners[1] >> winners[2]))
                return false;
            int prize = 0;
            row >> ws;
            if (!row.eof() && (!(row >> prize) || prize < 0))
                return false;
            Tournament cup(name, ids, prize);
            for (int i = 0; i < 3; i++)
                if (winners[i] != 0 && !cup.recordLegacyResult(i + 1, winners[i]))
                    return false;
            data.cups.push_back(cup);
        }
        else
            return false;

        string extra;
        if (row >> extra)
            return false;
    }
    return in.eof() && !in.bad();
}

} // file handling

// Every number prompt accepts 0 to go back.
int askNumber(const string& prompt, int low, int high)
{
    string line;
    while (cin)
    {
        cout << prompt;
        if (!getline(cin, line))
            break;
        istringstream input(line);
        int number;
        char extra;
        if (input >> number && !(input >> extra) &&
            (number == 0 || (number >= low && number <= high)))
            return number;

        cout << "Enter a whole number from " << low << " to " << high << ", or 0 to go back.\n";
    }
    return 0;
}

string askName(const string& what)
{
    cout << what << " name (0 to cancel): ";
    string name;
    if (!getline(cin, name))
        return "";
    size_t first = name.find_first_not_of(" \t\r");
    if (first == string::npos)
        return "";
    name.erase(name.find_last_not_of(" \t\r") + 1);
    name.erase(0, first);
    if (name == "0")
        return "";
    return name;
}

int choose(const vector<string>& names, const string& what)
{
    if (names.empty())
    {
        cout << "Create a " << what << " first.\n";
        return -1;
    }

    for (int i = 0; i < static_cast<int>(names.size()); i++)
        cout << i + 1 << ". " << names[i] << '\n';
    return askNumber("Choose a " + what + " (0 to go back): ", 1, names.size()) - 1;
}

vector<int> enterPlayers(int count, const PlayerRecords& players)
{
    vector<int> ids;
    if (count == 0)
        return ids;
    players.listPlayers();
    while (static_cast<int>(ids.size()) < count)
    {
        int id = askNumber("Player " + to_string(ids.size() + 1) + " ID (0 to cancel): ", 1, INT_MAX);
        if (id == 0)
            return {};
        if (!players.isActive(id))
            cout << "Choose an active player ID.\n";
        else if (find(ids.begin(), ids.end(), id) != ids.end())
            cout << "You have already entered that player.\n";
        else
            ids.push_back(id);
    }
    return ids;
}

namespace
{
string fixtureKey(char type, int eventNumber, int matchNumber)
{
    return string(1, type) + ":" + to_string(eventNumber) + ":" + to_string(matchNumber);
}

bool loadResults(Competition& event, char type, int eventNumber, const MatchHistory& history)
{
    // A saved match is the single record for the fixture, its stats and its prize.
    for (int i = 1; i <= static_cast<int>(event.matches().size()); i++)
    {
        const SavedMatch* saved = history.find(fixtureKey(type, eventNumber, i));
        if (saved == nullptr)
            continue;

        const Fixture& match = event.matches()[i - 1];
        const MatchResult& result = saved->result;
        if (result.count != 2 || result.playerIds[0] != match.first ||
            result.playerIds[1] != match.second)
            return false;
        if (match.winner != result.winnerId && !event.recordResult(i, result.winnerId))
            return false;
    }
    return true;
}

bool loadResults(Data& data, const MatchHistory& history)
{
    for (int i = 0; i < static_cast<int>(data.leagues.size()); i++)
        if (!loadResults(data.leagues[i], 'L', i + 1, history))
            return false;
    for (int i = 0; i < static_cast<int>(data.cups.size()); i++)
        if (!loadResults(data.cups[i], 'T', i + 1, history))
            return false;
    return true;
}

bool playFixture(Competition& event, int number, int score, int prize, const string& key,
                 PlayerRecords& players, MatchHistory& history, bool demo = false)
{
    const Fixture& match = event.matches()[number - 1];
    if (match.winner != 0 || match.first == 0 || match.second == 0)
    {
        cout << "Choose an unplayed fixture with two players.\n";
        return false;
    }
    if (!players.isActive(match.first) || !players.isActive(match.second))
    {
        cout << "Both fixture players must have active accounts.\n";
        return false;
    }

    int ids[2] = {match.first, match.second};
    string names[2] = {players.name(ids[0]), players.name(ids[1])};
    MatchResult result{};
    if (!playMatch(score, ids, names, 2, result, demo))
        return false;

    if (!history.record(result, players, key, prize))
    {
        cout << "The fixture result could not be saved.\n";
        return false;
    }
    event.recordResult(number, result.winnerId);
    cout << "Fixture result saved.\n";
    return true;
}

void fixtureMenu(Competition& event, char type, int eventNumber, int prize,
                 PlayerRecords& players, MatchHistory& history)
{
    cout << '\n' << event.getName() << '\n';
    const vector<Fixture>& matches = event.matches();
    bool open = false;
    for (int i = 0; i < static_cast<int>(matches.size()); i++)
    {
        const Fixture& match = matches[i];
        cout << i + 1 << ". ";
        if (type == 'T')
            cout << (i == 2 ? "Final: " : "Semifinal: ");
        if (match.first == 0 || match.second == 0)
            cout << "Waiting for both semifinal winners";
        else
        {
            cout << players.name(match.first) << " vs " << players.name(match.second);
            if (match.winner != 0)
                cout << " | Winner: " << players.name(match.winner);
            else
                open = true;
        }
        cout << '\n';
    }

    if (!open)
    {
        cout << "All fixtures are complete.\n";
        return;
    }
    int number = askNumber("Match to score (0 to go back): ", 1, matches.size());
    if (number == 0)
        return;
    const Fixture& match = matches[number - 1];
    if (match.first == 0 || match.second == 0 || match.winner != 0)
    {
        cout << "Choose an unplayed fixture with two players.\n";
        return;
    }

    cout << "1. Play 301\n2. Play 501\n0. Back\n";
    int mode = askNumber("Choice: ", 1, 2);
    if (mode == 0)
        return;
    int finalPrize = type == 'T' && number == 3 ? prize : 0;
    playFixture(event, number, mode == 1 ? 301 : 501, finalPrize,
                fixtureKey(type, eventNumber, number), players, history);
}

bool leagueAction(Data& data, int choice, PlayerRecords& players, MatchHistory& history)
{
    if (choice == 1)
    {
        string name = askName("League");
        if (name.empty())
            return false;
        int count = askNumber("How many players (2 to 8, 0 to cancel)? ", 2, MAX_LEAGUE_PLAYERS);
        vector<int> ids = enterPlayers(count, players);
        if (ids.empty())
            return false;
        data.leagues.emplace_back(name, ids);
        return true;
    }

    vector<string> names;
    for (const League& league : data.leagues)
        names.push_back(league.getName());
    int i = choose(names, "league");
    if (i < 0)
        return false;
    if (choice == 2)
        fixtureMenu(data.leagues[i], 'L', i + 1, 0, players, history);
    else
        data.leagues[i].showTable(data.clubs, players);
    return false;
}

void showChampion(const Tournament& cup, const PlayerRecords& players)
{
    if (cup.champion() == 0)
        return;

    ostringstream prize;
    prize << fixed << setprecision(2) << cup.getPrize() / 100.0;
    cout << "Tournament winner: " << players.name(cup.champion())
         << " | Prize: EUR " << prize.str() << '\n';
}

bool tournamentAction(Data& data, int choice, PlayerRecords& players,
                      MatchHistory& history, bool isAdmin)
{
    if (choice == 1)
    {
        string name = askName("Tournament");
        if (name.empty())
            return false;
        vector<int> ids = enterPlayers(4, players);
        if (ids.empty())
            return false;
        int prize = 0;
        if (isAdmin)
        {
            prize = 100 * askNumber("Prize in whole euros (0 for no prize): ", 1, 1000000);
            if (!cin)
                return false;
        }
        data.cups.emplace_back(name, ids, prize);
        return true;
    }

    vector<string> names;
    for (const Tournament& cup : data.cups)
        names.push_back(cup.getName());
    int i = choose(names, "tournament");
    if (i < 0)
        return false;

    Tournament& cup = data.cups[i];
    fixtureMenu(cup, 'T', i + 1, cup.getPrize(), players, history);
    showChampion(cup, players);
    return false;
}

bool playDemoFixtures(Competition& event, char type, int prize,
                      PlayerRecords& players, MatchHistory& history)
{
    for (int number = 1; number <= static_cast<int>(event.matches().size()); number++)
    {
        if (event.matches()[number - 1].winner != 0)
            continue;
        int finalPrize = type == 'T' && number == 3 ? prize : 0;
        if (!playFixture(event, number, type == 'T' ? 501 : 301, finalPrize,
                         fixtureKey(type, 1, number), players, history, true))
            return false;
    }
    return true;
}
} // fixture handling

void competitionMenu(int choice, PlayerRecords& players, MatchHistory& history,
                     bool isAdmin, const string& file)
{
    Data data;
    if (!loadData(data, file))
    {
        cout << "Could not read " << file << ". Check the save file before continuing.\n";
        return;
    }
    if (!loadResults(data, history))
    {
        cout << "Saved match history does not match the competition fixtures.\n";
        return;
    }
    while (cin)
    {
        string options;
        int last;
        if (choice == 5)
        {
            options = "\nCLUBS\n1. Create club\n2. Add member\n3. Remove member\n4. View club\n";
            last = 4;
        }
        else if (choice == 6)
        {
            options = "\nLEAGUES\n1. Create league\n2. Fixtures and results\n3. View table\n";
            last = 3;
        }
        else
        {
            options = "\nTOURNAMENTS\n1. Create tournament\n2. Draw and results\n";
            last = 2;
        }
        cout << options << "0. Back\n";
        int action = askNumber("Choice: ", 1, last);
        if (action == 0)
            return;

        bool changed;
        if (choice == 5)
            changed = clubAction(data.clubs, action, players);
        else if (choice == 6)
            changed = leagueAction(data, action, players, history);
        else
            changed = tournamentAction(data, action, players, history, isAdmin);

        if (changed)
        {
            if (!saveData(data, file))
            {
                cout << "Could not save this change. Check that the folder is writable.\n";
                return;
            }
            cout << "Saved.\n";
        }
    }
}

vector<Club> loadCompetitionClubs(const string& file)
{
    Data data;
    if (loadData(data, file))
        return data.clubs;
    return {};
}

string competitionClub(int playerId, const vector<Club>& clubs)
{
    for (const Club& club : clubs)
        if (club.hasPlayer(playerId))
            return club.getName();
    return "-";
}

bool runCompetitionDemo(PlayerRecords& players, MatchHistory& history,
                        const string& file, const int ids[4])
{
    Data data;
    if (!loadData(data, file))
        return false;

    if (data.clubs.empty())
    {
        data.clubs.emplace_back("River City Darts");
        for (int i = 0; i < 4; i++)
            data.clubs[0].addPlayer(ids[i]);
    }
    if (data.leagues.empty())
        data.leagues.emplace_back("Demo League", vector<int>(ids, ids + 3));
    if (data.cups.empty())
        data.cups.emplace_back("Demo Cup", vector<int>(ids, ids + 4), 5000);
    if (!saveData(data, file))
        return false;

    if (!loadResults(data, history) ||
        !playDemoFixtures(data.leagues[0], 'L', 0, players, history) ||
        !playDemoFixtures(data.cups[0], 'T', data.cups[0].getPrize(), players, history))
        return false;

    data.leagues[0].showTable(data.clubs, players);
    fixtureMenu(data.cups[0], 'T', 1, data.cups[0].getPrize(), players, history);
    showChampion(data.cups[0], players);
    return true;
}
