//score.cpp
#include <cctype>  //needed for toupper
#include "score.h"

using namespace std;

// Valid darts - 0 (miss), 1 - 20 with S/D/T, 25 (bullseye) with S or D only
bool isValidDart(int number, char multiplier)
{
    multiplier = toupper(multiplier);  // makes multiplier case insensitive

    if (multiplier != 'S' && multiplier != 'D' && multiplier != 'T') //have to have a multiplier selected
        return false;

    if (number == 0 || (number >= 1 && number <= 20))  // because numbers on board go from 1 to 20 and 0 is a miss
        return true;

    if (number == 25 && multiplier != 'T')   // because you cant get a triple bullseye
        return true;

    return false;
}

// Calculate individual dart score
int calculateDartScore(int number, char multiplier)
{
    multiplier = toupper(multiplier);

    switch (multiplier)  // switches states for multiples
    {
        case 'D': return number * 2;
        case 'T': return number * 3;
        default:  return number;
    }
}


// Checks remaining score on each dart so player finishes the game as
// soon as they win
DartResult applyDart(int& remScore, int dartScore, char multiplier)
{
    int newScore = remScore - dartScore;
    multiplier = toupper(multiplier);

    if (newScore == 0 && multiplier == 'D')
    {
        remScore = 0;
        return WIN;
    }

    // If score is below zero, left on 1, or hit 0 without a double player is bust
    if (newScore <= 1)
        return BUST;

    remScore = newScore;
    return CONTINUE;
}

namespace
{
const int MAX_DART_SCORE = 60;
const int MAX_VISIT_SCORE = 3 * MAX_DART_SCORE;

struct VisitScores
{
    bool totals[4][MAX_VISIT_SCORE + 1] = {};
    bool doubles[MAX_DART_SCORE + 1] = {};
    bool nonDoubles[MAX_DART_SCORE + 1] = {};

    VisitScores()
    {
        const char multipliers[] = {'S', 'D', 'T'};
        for (char multiplier : multipliers)
        {
            for (int number = 0; number <= 25; number++)
            {
                if (isValidDart(number, multiplier))
                {
                    int score = calculateDartScore(number, multiplier);
                    if (multiplier == 'D' && score > 0)
                        doubles[score] = true;
                    else
                        nonDoubles[score] = true;
                }
            }
        }

        totals[0][0] = true;
        for (int darts = 1; darts <= 3; darts++)
        {
            for (int total = 0; total <= (darts - 1) * MAX_DART_SCORE; total++)
            {
                if (!totals[darts - 1][total])
                    continue;
                for (int score = 0; score <= MAX_DART_SCORE; score++)
                {
                    if (doubles[score] || nonDoubles[score])
                        totals[darts][total + score] = true;
                }
            }
        }
    }
};

const VisitScores visitScores;
}

bool isPossibleVisit(int total, int darts)
{
    return darts >= 1 && darts <= 3 && total >= 0 && total <= MAX_VISIT_SCORE &&
           visitScores.totals[darts][total];
}

bool isValidVisit(int remaining, int total, int darts, bool finishedOnDouble)
{
    if (!isPossibleVisit(total, darts) || (finishedOnDouble && total != remaining))
        return false;
    if (remaining - total > 1)
        return true;

    // The first darts must leave at least two, so the visit only ends on its last dart.
    for (int last = 0; last <= MAX_DART_SCORE && last <= total; last++)
    {
        int earlierTotal = total - last;
        if (remaining - earlierTotal < 2 || !visitScores.totals[darts - 1][earlierTotal])
            continue;

        if (total != remaining && (visitScores.doubles[last] || visitScores.nonDoubles[last]))
            return true;
        if (total == remaining && finishedOnDouble && visitScores.doubles[last])
            return true;
        if (total == remaining && !finishedOnDouble && visitScores.nonDoubles[last])
            return true;
    }
    return false;
}

DartResult applyVisit(int& remaining, int total, bool finishedOnDouble)
{
    int nextScore = remaining - total;
    if (nextScore == 0 && finishedOnDouble)
    {
        remaining = 0;
        return WIN;
    }

    // Cancel the whole visit on a bust, including earlier darts in the visit.
    if (nextScore <= 1)
        return BUST;

    remaining = nextScore;
    return CONTINUE;
}
