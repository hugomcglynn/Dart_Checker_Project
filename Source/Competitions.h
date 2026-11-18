#ifndef COMPETITIONS_H
#define COMPETITIONS_H

#include <string>
#include <vector>

class PlayerRecords;
class MatchHistory;

struct Fixture
{
    int first;
    int second;
    int winner = 0;  // 0 means the match has not been played.
    int legacyWinner = 0;
};

// Both a league and a tournament keep a list of fixtures.
class Competition
{
public:
    Competition(std::string competitionName, const std::vector<int>& entrants);
    virtual ~Competition() = default;
    std::string getName() const;
    const std::vector<int>& entrants() const;
    const std::vector<Fixture>& matches() const;
    virtual bool recordResult(int matchNumber, int winnerId);
    bool recordLegacyResult(int matchNumber, int winnerId);

protected:
    std::vector<int> players;
    std::vector<Fixture> fixtures;

private:
    std::string name;
};

const int MAX_LEAGUE_PLAYERS = 8;
int askNumber(const std::string& prompt, int low, int high);
std::string askName(const std::string& what);
std::vector<int> enterPlayers(int count, const PlayerRecords& players);

int choose(const std::vector<std::string>& names, const std::string& what);
void competitionMenu(int choice, PlayerRecords& players, MatchHistory& history,
                     bool isAdmin, const std::string& file);
bool runCompetitionDemo(PlayerRecords& players, MatchHistory& history,
                        const std::string& file, const int ids[4]);

#endif
