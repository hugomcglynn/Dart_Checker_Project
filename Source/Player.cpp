#include "Player.h"

Player::Player(int playerId, const std::string& playerName, int playerAge,
               const std::string& playerNationality, const std::string& playerAddress,
               const std::string& playerPin, bool activePlayer)
    : id(playerId), name(playerName), age(playerAge), nationality(playerNationality),
      address(playerAddress), pin(playerPin), active(activePlayer)
{
}

int Player::getId() const
{
    return id;
}

const std::string& Player::getName() const
{
    return name;
}

bool Player::isActive() const
{
    return active;
}
