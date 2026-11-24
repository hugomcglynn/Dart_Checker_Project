// Brendan Galvin supplied the original club code.
// Codex integrated it with the shared player and match records.

#include "Club.h"
#include "Competitions.h"
#include "PlayerRecords.h"
#include <climits>
#include <iostream>
#include <iomanip>
using namespace std;

Club::Club(string clubName) : name(clubName) {}

int Club::memberIndex(int playerId) const
{
    for (int i = 0; i < playerCount; i++)
        if (playerIds[i] == playerId)
            return i;

    return -1;
}

bool Club::hasPlayer(int playerId) const
{
    return memberIndex(playerId) >= 0;
}

bool Club::addPlayer(int playerId)
{
    if (playerId <= 0)
        cout << "Player IDs must be positive.\n";
    else if (hasPlayer(playerId))
        cout << "Player is already in this club.\n";
    else if (playerCount == MAX_CLUB_PLAYERS)
        cout << "Club is full.\n";
    else
    {
        playerIds[playerCount] = playerId;
        playerCount++;
        return true;
    }
    return false;
}

bool Club::removePlayer(int playerId)
{
    int index = memberIndex(playerId);
    if (index < 0)
    {
        cout << "Player is not in this club.\n";
        return false;
    }

    for (int i = index; i < playerCount - 1; i++)
        playerIds[i] = playerIds[i + 1];
    playerCount--;
    return true;
}

void Club::displayClub(const PlayerRecords& players) const
{
    cout << "Club: " << name << "\nNumber of players: " << playerCount << "\nPlayers:\n";
    for (int i = 0; i < playerCount; i++)
        cout << playerIds[i] << "  " << players.name(playerIds[i]) << '\n';
}

string Club::getName() const
{
    return name;
}

void Club::save(ostream& out) const
{
    out << "C " << quoted(name) << ' ' << playerCount;
    for (int i = 0; i < playerCount; i++)
        out << ' ' << playerIds[i];
    out << '\n';
}

static const Club* clubForPlayer(int playerId, const vector<Club>& clubs)
{
    for (const Club& club : clubs)
        if (club.hasPlayer(playerId))
            return &club;
    return nullptr;
}

string competitionClub(int playerId, const vector<Club>& clubs)
{
    const Club* club = clubForPlayer(playerId, clubs);
    if (club == nullptr)
        return "-";
    return club->getName();
}

bool clubAction(vector<Club>& clubs, int choice, const PlayerRecords& players)
{
    if (choice == 1)
    {
        string name = askName("Club");
        if (name.empty())
            return false;
        clubs.push_back(Club(name));
        return true;
    }

    vector<string> names;
    for (const Club& item : clubs)
        names.push_back(item.getName());
    int i = choose(names, "club");
    if (i < 0)
        return false;
    Club& club = clubs[i];
    if (choice == 2)
        players.listPlayers();
    else
        club.displayClub(players);
    if (choice == 4)
        return false;

    int id = askNumber("Player ID (0 to cancel): ", 1, INT_MAX);
    if (id == 0)
        return false;
    if (choice == 3)
        return club.removePlayer(id);

    if (!players.isActive(id))
    {
        cout << "Choose an active player ID.\n";
        return false;
    }
    const Club* currentClub = clubForPlayer(id, clubs);
    if (currentClub != nullptr)
    {
        cout << "Player " << id << " already belongs to " << currentClub->getName() << ".\n";
        return false;
    }
    return club.addPlayer(id);
}
