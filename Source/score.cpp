// Hugo McGlynn wrote single-dart scoring. Brendan Galvin fixed the include.
// Codex added visit scoring.

#include "score.h"
#include <cctype>

// The bull scores 25 or 50. A miss scores zero.
bool isValidDart(int number, char multiplier)
{
    multiplier = std::toupper(static_cast<unsigned char>(multiplier));

    if (multiplier != 'S' && multiplier != 'D' && multiplier != 'T')
        return false;
    if (number >= 0 && number <= 20)
        return true;
    return number == 25 && multiplier != 'T';
}

int calculateDartScore(int number, char multiplier)
{
    switch (std::toupper(static_cast<unsigned char>(multiplier)))
    {
        case 'D': return number * 2;
        case 'T': return number * 3;
        default:  return number;
    }
}

DartResult applyDart(int& remScore, int dartScore, char multiplier)
{
    int newScore = remScore - dartScore;
    multiplier = std::toupper(static_cast<unsigned char>(multiplier));

    if (newScore == 0 && multiplier == 'D')
    {
        remScore = 0;
        return WIN;
    }
    if (newScore <= 1)
        return BUST;

    remScore = newScore;
    return CONTINUE;
}

const int MAX_DART_SCORE = 60;

static bool isDoubleScore(int score)
{
    return score > 0 && score % 2 == 0 && isValidDart(score / 2, 'D');
}

static bool isNonDoubleScore(int score)
{
    return isValidDart(score, 'S') || (score % 3 == 0 && isValidDart(score / 3, 'T'));
}

static bool isDartScore(int score)
{
    return isDoubleScore(score) || isNonDoubleScore(score);
}

static bool validVisitRange(int total, int darts)
{
    return darts >= 1 && darts <= 3 && total >= 0 && total <= darts * MAX_DART_SCORE;
}

// A visit has at most two darts before its last dart.
static bool canScoreBeforeLast(int total, int darts)
{
    if (darts == 0)
        return total == 0;
    if (darts == 1)
        return isDartScore(total);
    if (total > 2 * MAX_DART_SCORE)
        return false;

    for (int first = 0; first <= MAX_DART_SCORE && first <= total; first++)
    {
        if (isDartScore(first) && isDartScore(total - first))
            return true;
    }
    return false;
}

bool isPossibleVisit(int total, int darts)
{
    if (!validVisitRange(total, darts))
        return false;

    for (int last = 0; last <= MAX_DART_SCORE && last <= total; last++)
    {
        if (isDartScore(last) && canScoreBeforeLast(total - last, darts - 1))
            return true;
    }
    return false;
}

bool isValidVisit(int remaining, int total, int darts, bool finishedOnDouble)
{
    if (!validVisitRange(total, darts) || remaining < 2)
        return false;
    if (finishedOnDouble && total != remaining)
        return false;
    if (remaining - total > 1)
        return isPossibleVisit(total, darts);

    // Until the last dart there must still be at least two points left.
    for (int last = 0; last <= MAX_DART_SCORE && last <= total; last++)
    {
        int beforeLast = total - last;
        if (remaining - beforeLast < 2)
            continue;

        bool lastDartIsLegal = isDartScore(last);
        if (total == remaining && finishedOnDouble)
            lastDartIsLegal = isDoubleScore(last);
        if (total == remaining && !finishedOnDouble)
            lastDartIsLegal = isNonDoubleScore(last);

        if (lastDartIsLegal && canScoreBeforeLast(beforeLast, darts - 1))
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

    // A bust leaves the score unchanged for the whole visit.
    if (nextScore <= 1)
        return BUST;

    remaining = nextScore;
    return CONTINUE;
}
