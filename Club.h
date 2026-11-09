#ifndef CLUB_H
#define CLUB_H

#include <string>

const int MAX_CLUB_PLAYERS = 20;

class Club
{
public:
    Club(std::string clubName);
    void addPlayer(int playerId);
    void displayClub();

private:
    std::string name;
    int playerIds[MAX_CLUB_PLAYERS];
    int playerCount;
};

#endif
