#include "Club.h"
#include <iostream>
using namespace std;

Club::Club(string clubName)
{
    name = clubName;
    playerCount = 0;
}

void Club::addPlayer(int playerId)
{
    if (playerCount == MAX_CLUB_PLAYERS)
    {
        cout << "Club is full.\n";
        return;
    }

    for (int i = 0; i < playerCount; i++)
        if (playerIds[i] == playerId)
        {
            cout << "Player is already in this club.\n";
            return;
        }

    playerIds[playerCount] = playerId;
    playerCount++;
}

void Club::displayClub()
{
    cout << "Club: " << name << "\n";
    cout << "Number of players: " << playerCount << "\n";
    cout << "Player IDs:\n";

    for (int i = 0; i < playerCount; i++)
        cout << playerIds[i] << "\n";
}
