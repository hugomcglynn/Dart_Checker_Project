// Brendan Galvin supplied the original tournament code.
// Codex integrated it with the shared player and match records.

#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "Competitions.h"

class Tournament : public Competition
{
public:
    Tournament(std::string name, const std::vector<int>& players, int prize = 0);
    bool recordResult(int matchNumber, int winnerId) override;
    int champion() const;
    int getPrize() const;

private:
    int prizeCents;
};

#endif
