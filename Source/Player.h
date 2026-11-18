#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player
{
public:
    Player(int playerId, const std::string& playerName, int playerAge,
           const std::string& playerNationality, const std::string& playerAddress,
           const std::string& playerPin, bool activePlayer = true);
    int getId() const;
    const std::string& getName() const;
    bool isActive() const;

private:
    friend class PlayerRecords;

    int id;
    std::string name;
    int age;
    std::string nationality;
    std::string address;
    std::string pin;
    bool active;
};

#endif
