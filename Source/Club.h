#ifndef CLUB_H
#define CLUB_H

#include <iosfwd>
#include <string>
#include <vector>

class PlayerRecords;

const int MAX_CLUB_PLAYERS = 20;

class Club
{
public:
    Club(std::string clubName);
    void addPlayer(int playerId);
    void removePlayer(int playerId);
    void displayClub(const PlayerRecords& players) const;
    bool hasPlayer(int playerId) const;

    std::string getName() const;
    void save(std::ostream& out) const;

private:
    std::string name;
    int playerIds[MAX_CLUB_PLAYERS] = {};
    int playerCount = 0;
};

bool clubAction(std::vector<Club>& clubs, int choice, const PlayerRecords& players);
std::vector<Club> loadCompetitionClubs(const std::string& file);
std::string competitionClub(int playerId, const std::vector<Club>& clubs);

#endif
