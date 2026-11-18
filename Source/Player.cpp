#include "Player.h"

Player::Player(int playerId, const std::string& playerName, int playerAge,
               const std::string& playerNationality, const std::string& playerAddress,
               const std::string& hash, bool activePlayer, bool needsPassword)
    : id(playerId), name(playerName), age(playerAge), nationality(playerNationality),
      address(playerAddress), passwordHash(hash), active(activePlayer), needsNewPassword(needsPassword)
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
