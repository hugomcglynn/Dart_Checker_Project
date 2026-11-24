// Codex declared gameplay for Hugo McGlynn's part of the team plan.

#ifndef GAME_H
#define GAME_H

#include <string>

const int MAX_MATCH_PLAYERS = 8;

struct MatchResult
{
    int startingScore;
    int playerIds[MAX_MATCH_PLAYERS];
    int remaining[MAX_MATCH_PLAYERS];
    int darts[MAX_MATCH_PLAYERS];
    int winnerId;
    int count;
};

bool playMatch(int start, const int ids[], const std::string names[], int count,
               MatchResult& result, bool demo = false);

#endif
