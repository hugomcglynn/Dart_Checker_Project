#include "Tournament.h"
using namespace std;

Tournament::Tournament(string name, const vector<int>& entrants, int prize)
    : Competition(name, entrants), prizeCents(prize)
{
    if (players.size() == 4)
    {
        fixtures.push_back({players[0], players[1]});
        fixtures.push_back({players[2], players[3]});
        fixtures.push_back({0, 0});
    }
}

bool Tournament::recordResult(int matchNumber, int winnerId)
{
    if (!Competition::recordResult(matchNumber, winnerId))
        return false;

    // The two semifinal winners get the places in the final.
    fixtures[2].first = fixtures[0].winner;
    fixtures[2].second = fixtures[1].winner;
    return true;
}

int Tournament::champion() const
{
    if (fixtures.size() < 3)
        return 0;
    return fixtures[2].winner;
}

int Tournament::getPrize() const
{
    return prizeCents;
}
