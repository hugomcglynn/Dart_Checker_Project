// Gary's records area. Implemented by Codex.

#include "MatchHistory.h"
#include "Club.h"
#include "Competitions.h"
#include "PlayerRecords.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

using namespace std;

namespace
{
bool validMatch(const MatchResult& result)
{
    if ((result.startingScore != 301 && result.startingScore != 501)
        || result.count < 2 || result.count > MAX_MATCH_PLAYERS)
        return false;

    bool hasWinner = false;
    for (int i = 0; i < result.count; ++i)
    {
        if (result.playerIds[i] <= 0 || result.remaining[i] < 0
            || result.remaining[i] > result.startingScore || result.remaining[i] == 1
            || result.darts[i] < -1)
            return false;
        for (int j = 0; j < i; ++j)
            if (result.playerIds[i] == result.playerIds[j])
                return false;
        bool won = result.playerIds[i] == result.winnerId;
        if (won != (result.remaining[i] == 0))
            return false;
        if (won)
            hasWinner = true;
    }
    return hasWinner;
}

struct Standing
{
    int id;
    int previousRank = 0;
    int wins = 0;
    int losses = 0;
    long long points = 0;
    long long pointsWithDartCount = 0;
    long long darts = 0;
    long long prizeCents = 0;
};

double average(const Standing& player)
{
    return player.darts == 0 ? 0 : 3.0 * player.pointsWithDartCount / player.darts;
}

bool ranksHigher(const Standing& first, const Standing& second)
{
    if (first.wins != second.wins)
        return first.wins > second.wins;
    double firstAverage = average(first);
    double secondAverage = average(second);
    if (firstAverage != secondAverage)
        return firstAverage > secondAverage;
    return first.id < second.id;
}

void sortStandings(vector<Standing>& table)
{
    for (size_t i = 0; i < table.size(); ++i)
    {
        size_t best = i;
        for (size_t j = i + 1; j < table.size(); ++j)
            if (ranksHigher(table[j], table[best]))
                best = j;
        swap(table[i], table[best]);
    }
}

void addMatch(Standing& player, const SavedMatch& match)
{
    const MatchResult& result = match.result;
    for (int i = 0; i < result.count; ++i)
    {
        if (player.id != result.playerIds[i])
            continue;

        int points = result.startingScore - result.remaining[i];
        player.points += points;
        // An old match without a dart count still earns points but won't affect the average.
        if (result.darts[i] >= 0)
        {
            player.pointsWithDartCount += points;
            player.darts += result.darts[i];
        }
        if (player.id == result.winnerId)
        {
            ++player.wins;
            player.prizeCents += match.prizeCents;
        }
        else
            ++player.losses;
        return;
    }
}

const SavedMatch* findFixture(const vector<SavedMatch>& matches, const string& key)
{
    if (key.empty())
        return nullptr;
    for (const SavedMatch& match : matches)
        if (match.fixtureKey == key)
            return &match;
    return nullptr;
}
}

MatchHistory::MatchHistory(const string& filename) : filename(filename) {}

bool MatchHistory::load()
{
    ifstream input(filename);
    if (!input)
    {
        error_code error;
        return !filesystem::exists(filename, error) && !error;
    }

    vector<SavedMatch> loaded;
    string line;
    while (getline(input, line))
    {
        if (line.find_first_not_of(" \t\r") == string::npos)
            continue;
        istringstream row(line);
        SavedMatch match;
        MatchResult& result = match.result;
        int version;
        if (!(row >> version >> quoted(match.fixtureKey) >> match.prizeCents
                  >> result.startingScore >> result.winnerId >> result.count)
            || (version != 1 && version != 2) || match.prizeCents < 0
            || result.count < 2 || result.count > MAX_MATCH_PLAYERS)
            return false;

        for (int i = 0; i < result.count; ++i)
        {
            if (!(row >> result.playerIds[i] >> result.remaining[i]))
                return false;
            result.darts[i] = -1;
            if (version == 2 && !(row >> result.darts[i]))
                return false;
            if (!(row >> quoted(match.names[i])))
                return false;
        }
        string extra;
        if (!validMatch(result) || (row >> extra) || findFixture(loaded, match.fixtureKey))
            return false;
        loaded.push_back(match);
    }
    if (input.bad())
        return false;
    matches.swap(loaded);
    return true;
}

bool MatchHistory::save() const
{
    string temporary = filename + ".tmp";
    ofstream output(temporary);
    if (!output)
        return false;
    for (const SavedMatch& match : matches)
    {
        const MatchResult& result = match.result;
        output << 2 << ' ' << quoted(match.fixtureKey) << ' ' << match.prizeCents
               << ' ' << result.startingScore << ' ' << result.winnerId << ' ' << result.count;
        for (int i = 0; i < result.count; ++i)
            output << ' ' << result.playerIds[i] << ' ' << result.remaining[i]
                   << ' ' << result.darts[i] << ' ' << quoted(match.names[i]);
        output << '\n';
    }
    output.close();
    if (!output)
        return false;

    error_code error;
    filesystem::rename(temporary, filename, error);
    return !error;
}

const SavedMatch* MatchHistory::find(const string& fixtureKey) const
{
    return findFixture(matches, fixtureKey);
}

bool MatchHistory::record(const MatchResult& result, const PlayerRecords& players,
                          const string& fixtureKey, int prizeCents)
{
    if (!validMatch(result) || prizeCents < 0 || find(fixtureKey))
        return false;

    SavedMatch match;
    match.result = result;
    match.fixtureKey = fixtureKey;
    match.prizeCents = prizeCents;
    for (int i = 0; i < result.count; ++i)
    {
        if (!players.isActive(result.playerIds[i]))
            return false;
        match.names[i] = players.name(result.playerIds[i]);
    }

    matches.push_back(match);
    if (save())
        return true;
    matches.pop_back();
    cerr << "Could not save the match record.\n";
    return false;
}

void MatchHistory::show() const
{
    cout << "\nGame History\n";
    if (matches.empty())
        cout << "No completed matches.\n";
    for (int i = 0; i < static_cast<int>(matches.size()); ++i)
    {
        const SavedMatch& match = matches[i];
        const MatchResult& result = match.result;
        cout << "\nMatch " << i + 1 << " (" << result.startingScore << ')';
        if (!match.fixtureKey.empty())
            cout << " Fixture " << match.fixtureKey;
        cout << "\nName (ID) | Remaining | Credited | Darts\n";
        for (int j = 0; j < result.count; ++j)
        {
            cout << match.names[j] << " (#" << result.playerIds[j] << ") | "
                 << result.remaining[j] << " | " << result.startingScore - result.remaining[j] << " | ";
            if (result.darts[j] >= 0)
                cout << result.darts[j];
            if (result.playerIds[j] == result.winnerId)
                cout << "  Winner";
            cout << '\n';
        }
    }
}

void MatchHistory::showPlayerStats(int playerId) const
{
    Standing player{playerId};
    for (const SavedMatch& match : matches)
        addMatch(player, match);
    cout << "Wins: " << player.wins << "\nLosses: " << player.losses
         << "\nCredited points: " << player.points << '\n';
}

void MatchHistory::showLeaderboard(const PlayerRecords& players, const string& competitionFile) const
{
    vector<Standing> table;
    for (int id : players.activeIds())
    {
        Standing player{id};
        for (size_t i = 0; i + 1 < matches.size(); ++i)
            addMatch(player, matches[i]);
        table.push_back(player);
    }
    if (table.empty())
    {
        cout << "\nNo active players.\n";
        return;
    }

    // Rank the players before and after adding the most recent match.
    sortStandings(table);
    for (size_t i = 0; i < table.size(); ++i)
    {
        table[i].previousRank = static_cast<int>(i) + 1;
        if (!matches.empty())
            addMatch(table[i], matches.back());
    }
    sortStandings(table);
    const vector<Club> clubs = loadCompetitionClubs(competitionFile);

    cout << "\nLeaderboard\n" << left
         << setw(6) << "Rank" << setw(8) << "Move" << setw(23) << "Player"
         << setw(21) << "Club" << right << setw(8) << "3-dart" << setw(6) << "Wins"
         << setw(8) << "Losses" << setw(9) << "Points" << setw(13) << "Prize (EUR)" << '\n';
    for (size_t i = 0; i < table.size(); ++i)
    {
        int rank = static_cast<int>(i) + 1;
        const Standing& player = table[i];
        int movement = player.previousRank - rank;
        string move = movement > 0 ? "+" + to_string(movement) : to_string(movement);
        string club = competitionClub(player.id, clubs);
        cout << left << setw(6) << rank << setw(8) << move
             << setw(23) << players.name(player.id).substr(0, 21)
             << setw(21) << club.substr(0, 19) << right << fixed << setprecision(2);
        if (player.darts > 0)
            cout << setw(8) << average(player);
        else
            cout << setw(8) << "-";
        cout << setw(6) << player.wins << setw(8) << player.losses << setw(9) << player.points
             << setw(13) << player.prizeCents / 100.0 << '\n';
    }
    cout << defaultfloat << setprecision(6);
}
