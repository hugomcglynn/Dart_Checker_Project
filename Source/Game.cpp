// Hugo McGlynn is responsible for gameplay in the team plan.
// Codex implemented this file using Hugo's original dart scoring.

#include "Game.h"
#include "score.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>

using namespace std;

namespace
{
struct Visit
{
    int total = 0;
    int darts = 3;
    bool finishedOnDouble = false;
};

bool readVisit(int remaining, Visit& visit)
{
    string line;
    while (cin)
    {
        visit = Visit{};
        cout << "Visit total (0-180, total/darts for a short visit, q to abandon): ";
        if (!getline(cin, line) || line == "q" || line == "Q")
            return false;

        istringstream input(line);
        char extra;
        bool validInput = static_cast<bool>(input >> visit.total);
        bool enteredDarts = validInput && (input >> extra);
        if (enteredDarts)
            validInput = extra == '/' && (input >> visit.darts) && !(input >> extra);
        if (!validInput || !isPossibleVisit(visit.total, visit.darts))
        {
            cout << "Enter a total possible with at most three darts.\n";
            continue;
        }

        if (visit.total == remaining)
        {
            while (true)
            {
                cout << "Was the last dart a double? (y/n): ";
                if (!getline(cin, line) || line == "q" || line == "Q")
                    return false;
                if (line == "y" || line == "Y" || line == "n" || line == "N")
                    break;
            }
            visit.finishedOnDouble = line == "y" || line == "Y";
        }

        if (remaining - visit.total <= 1 && !enteredDarts)
        {
            while (true)
            {
                cout << "Darts thrown (1-3): ";
                if (!getline(cin, line) || line == "q" || line == "Q")
                    return false;
                istringstream dartsInput(line);
                if (dartsInput >> visit.darts && !(dartsInput >> extra) &&
                    visit.darts >= 1 && visit.darts <= 3)
                    break;
            }
        }

        if (isValidVisit(remaining, visit.total, visit.darts, visit.finishedOnDouble))
            return true;

        cout << "That total, dart count and finish cannot occur in one visit. Re-enter the visit.\n";
    }
    return false;
}

Visit demoVisit(int remaining)
{
    if (remaining <= 100)
    {
        for (int darts = 1; darts <= 3; darts++)
        {
            if (isValidVisit(remaining, remaining, darts, true))
                return Visit{remaining, darts, true};
        }

        // A single sets up a lower checkout. The other two darts are misses.
        return Visit{min(20, remaining - 2), 3, false};
    }

    static mt19937 generator(301);
    uniform_int_distribution<int> target(15, 20);
    int total = calculateDartScore(target(generator), 'T');
    total += calculateDartScore(target(generator), 'S');
    total += calculateDartScore(target(generator), 'S');
    return Visit{total, 3, false};
}
}

bool playMatch(int start, const int ids[], const string names[], int count,
               MatchResult& result, bool demo)
{
    if ((start != 301 && start != 501) || count < 2 || count > 8)
        return false;

    result = MatchResult{};
    result.startingScore = start;
    result.count = count;
    for (int i = 0; i < count; i++)
    {
        result.playerIds[i] = ids[i];
        result.remaining[i] = start;
    }

    cout << "\nStarting " << start << " match.\n";
    while (true)
    {
        for (int i = 0; i < count; i++)
        {
            int& remaining = result.remaining[i];
            cout << names[i] << " has " << remaining << " remaining.\n";
            Visit visit;
            if (demo)
            {
                visit = demoVisit(remaining);
                cout << "Visit total " << visit.total << " in " << visit.darts << " dart(s).\n";
            }
            else if (!readVisit(remaining, visit))
            {
                cout << "Match abandoned.\n";
                return false;
            }

            result.darts[i] += visit.darts;
            DartResult outcome = applyVisit(remaining, visit.total, visit.finishedOnDouble);
            if (outcome == BUST)
                cout << "Bust! " << names[i] << " stays on " << remaining << ".\n";
            else if (outcome == WIN)
            {
                result.winnerId = ids[i];
                cout << names[i] << " wins!\n";
                return true;
            }
            else
                cout << names[i] << " is now on " << remaining << ".\n";
        }
    }
}
