// Codex implemented gameplay for Hugo McGlynn's part of the team plan.

#include "Game.h"
#include "score.h"

#include <iostream>
#include <random>
#include <sstream>

using namespace std;

struct Visit
{
    int total = 0;
    int darts = 3;
    bool finishedOnDouble = false;
};

static bool readAnswer(const char* prompt, string& answer)
{
    cout << prompt;
    return getline(cin, answer) && answer != "q" && answer != "Q";
}

static bool readDouble(bool& finishedOnDouble)
{
    string answer;
    while (readAnswer("Was the last dart a double? (y/n): ", answer))
    {
        if (answer == "y" || answer == "Y")
        {
            finishedOnDouble = true;
            return true;
        }
        if (answer == "n" || answer == "N")
        {
            finishedOnDouble = false;
            return true;
        }
    }
    return false;
}

static bool readDartCount(int& darts)
{
    string answer;
    while (readAnswer("Darts thrown (1-3): ", answer))
    {
        istringstream input(answer);
        char extra;
        if (input >> darts && !(input >> extra) && darts >= 1 && darts <= 3)
            return true;
    }
    return false;
}

static bool parseTotal(const string& line, Visit& visit, bool& gaveDartCount)
{
    istringstream input(line);
    if (!(input >> visit.total))
        return false;

    char separator;
    if (!(input >> separator))
        return true;

    gaveDartCount = true;
    return separator == '/' && (input >> visit.darts) && !(input >> separator);
}

static bool readVisit(int remaining, Visit& visit)
{
    string line;
    while (readAnswer("Visit total (0-180, total/darts for a short visit, q to abandon): ", line))
    {
        visit = Visit{};
        bool gaveDartCount = false;
        if (!parseTotal(line, visit, gaveDartCount) || !isPossibleVisit(visit.total, visit.darts))
        {
            cout << "Enter a total possible with at most three darts.\n";
            continue;
        }

        if (visit.total == remaining && !readDouble(visit.finishedOnDouble))
            return false;
        if (remaining - visit.total <= 1 && !gaveDartCount && !readDartCount(visit.darts))
            return false;

        if (isValidVisit(remaining, visit.total, visit.darts, visit.finishedOnDouble))
            return true;

        cout << "That total, dart count and finish cannot occur in one visit. Re-enter the visit.\n";
    }
    return false;
}

static Visit demoVisit(int remaining)
{
    if (remaining <= 100)
    {
        for (int darts = 1; darts <= 3; darts++)
        {
            if (isValidVisit(remaining, remaining, darts, true))
                return Visit{remaining, darts, true};
        }

        // A single and two misses set up a lower checkout.
        int total = remaining - 2;
        if (total > 20)
            total = 20;
        return Visit{total, 3, false};
    }

    static mt19937 generator(301);
    uniform_int_distribution<int> target(15, 20);
    int total = calculateDartScore(target(generator), 'T');
    total += calculateDartScore(target(generator), 'S');
    total += calculateDartScore(target(generator), 'S');
    return Visit{total, 3, false};
}

bool playMatch(int start, const int ids[], const string names[], int count,
               MatchResult& result, bool demo)
{
    if ((start != 301 && start != 501) || count < 2 || count > MAX_MATCH_PLAYERS)
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
    int player = 0;
    while (true)
    {
        cout << names[player] << " has " << result.remaining[player] << " remaining.\n";
        Visit visit;
        if (demo)
        {
            visit = demoVisit(result.remaining[player]);
            cout << "Visit total " << visit.total << " in " << visit.darts << " dart(s).\n";
        }
        else if (!readVisit(result.remaining[player], visit))
        {
            cout << "Match abandoned.\n";
            return false;
        }

        result.darts[player] += visit.darts;
        DartResult outcome = applyVisit(result.remaining[player], visit.total, visit.finishedOnDouble);
        if (outcome == WIN)
        {
            result.winnerId = ids[player];
            cout << names[player] << " wins!\n";
            return true;
        }
        if (outcome == BUST)
            cout << "Bust! " << names[player] << " stays on " << result.remaining[player] << ".\n";
        else
            cout << names[player] << " is now on " << result.remaining[player] << ".\n";

        player++;
        if (player == count)
            player = 0;
    }
}
