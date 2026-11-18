// Gary is responsible for match history and player statistics in the team plan.
// Codex implemented this file during integration.

#ifndef MATCH_HISTORY_H
#define MATCH_HISTORY_H

#include "Game.h"
#include <string>
#include <vector>

class PlayerRecords;

struct SavedMatch
{
    MatchResult result{};
    std::string names[8];
    std::string fixtureKey;
    int prizeCents = 0;
};

class MatchHistory
{
public:
    explicit MatchHistory(const std::string& filename);
    bool load();
    const SavedMatch* find(const std::string& fixtureKey) const;
    bool record(const MatchResult& result, const PlayerRecords& players,
                const std::string& fixtureKey = "", int prizeCents = 0);
    void show() const;
    void showPlayerStats(int playerId) const;
    void showLeaderboard(const PlayerRecords& players, const std::string& competitionFile) const;

private:
    bool save() const;
    std::string filename;
    std::vector<SavedMatch> matches;
};

#endif
