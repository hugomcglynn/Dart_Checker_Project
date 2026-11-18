// Hugo McGlynn is responsible for gameplay in the team plan.
// Codex implemented this file using Hugo's original dart scoring.

#ifndef GAME_H
#define GAME_H

#include <string>

struct MatchResult
{
    int startingScore;
    int playerIds[8];
    int remaining[8];
    int darts[8];
    int winnerId;
    int count;
};

bool playMatch(int start, const int ids[], const std::string names[], int count,
               MatchResult& result, bool demo = false);

#endif
