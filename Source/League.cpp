#include "League.h"
#include "PlayerRecords.h"
#include <iostream>
#include <iomanip>
using namespace std;

League::League(string name, const vector<int>& entrants) : Competition(name, entrants)
{
    // Starting j after i gives each pair exactly one match.
    for (int i = 0; i < static_cast<int>(players.size()); i++)
        for (int j = i + 1; j < static_cast<int>(players.size()); j++)
            fixtures.push_back({players[i], players[j]});
}

void League::showTable(const vector<Club>& clubs, const PlayerRecords& records) const
{
    struct Row { int id, played, won; };
    vector<Row> table;

    for (int id : players)
    {
        Row row = {id, 0, 0};
        for (const Fixture& match : fixtures)
            if (match.winner != 0 && (match.first == id || match.second == id))
            {
                row.played++;
                if (match.winner == id)
                    row.won++;
            }
        table.push_back(row);
    }

    cout << "\n" << getName() << "\n";
    cout << setw(10) << "Player ID" << setw(8) << "Played" << setw(6) << "Won"
         << setw(6) << "Lost" << setw(8) << "Points" << "  Player | Club\n";

    // Most wins first. Players level on points stay in entry order.
    for (int wins = static_cast<int>(players.size()) - 1; wins >= 0; wins--)
        for (const Row& row : table)
            if (row.won == wins)
            {
                cout << setw(10) << row.id << setw(8) << row.played << setw(6) << row.won
                     << setw(6) << row.played - row.won << setw(8) << 2 * row.won
                     << "  " << records.name(row.id) << " | " << competitionClub(row.id, clubs) << '\n';
            }
}
