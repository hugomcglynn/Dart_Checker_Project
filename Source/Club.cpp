#include "Club.h"
#include "Competitions.h"
#include "PlayerRecords.h"
#include <climits>
#include <iostream>
#include <iomanip>
using namespace std;

Club::Club(string clubName)
{
    name = clubName;
}

bool Club::hasPlayer(int playerId) const
{
    for (int i = 0; i < playerCount; i++)
        if (playerIds[i] == playerId)
            return true;

    return false;
}

void Club::addPlayer(int playerId)
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
    }
}

void Club::removePlayer(int playerId)
{
    for (int i = 0; i < playerCount; i++)
        if (playerIds[i] == playerId)
        {
            for (int j = i; j < playerCount - 1; j++)
                playerIds[j] = playerIds[j + 1];
            playerCount--;
            return;
        }

    cout << "Player is not in this club.\n";
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
    if (choice == 4)
    {
        club.displayClub(players);
        return false;
    }

    if (choice == 2)
        players.listPlayers();
    else
        club.displayClub(players);
    int id = askNumber("Player ID (0 to cancel): ", 1, INT_MAX);
    if (id == 0)
        return false;
    if (choice == 2)
    {
        if (!players.isActive(id))
        {
            cout << "Choose an active player ID.\n";
            return false;
        }
        for (const Club& other : clubs)
            if (other.hasPlayer(id))
            {
                cout << "Player " << id << " already belongs to " << other.getName() << ".\n";
                return false;
            }
        club.addPlayer(id);
        return club.hasPlayer(id);
    }

    bool wasMember = club.hasPlayer(id);
    club.removePlayer(id);
    return wasMember;
}
