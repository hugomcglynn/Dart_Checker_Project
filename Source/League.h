#ifndef LEAGUE_H
#define LEAGUE_H

#include "Club.h"
#include "Competitions.h"

class League : public Competition
{
public:
    League(std::string name, const std::vector<int>& players);
    void showTable(const std::vector<Club>& clubs, const PlayerRecords& players) const;
};

#endif
