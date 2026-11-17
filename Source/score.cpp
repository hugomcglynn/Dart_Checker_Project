//score.cpp
#include <cctype>  //needed for toupper
#include "Score.h"

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
