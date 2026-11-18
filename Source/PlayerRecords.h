#ifndef PLAYER_RECORDS_H
#define PLAYER_RECORDS_H

#include "Player.h"
#include <string>
#include <vector>

class MatchHistory;

class PlayerRecords
{
public:
    explicit PlayerRecords(const std::string& filename);
    bool load();
    const Player* find(int id) const;
    bool isActive(int id) const;
    int lastId() const;
    std::string name(int id) const;
    void listPlayers() const;
    int selectPlayer(bool allowCreate = false);
    int addPlayer(const std::string& name, int age, const std::string& nationality,
                  const std::string& address, const std::string& pin);

    bool isAdmin() const;
    int signedInPlayer() const;
    void accountMenu(const MatchHistory& history);
    bool showProfile(int id) const;

private:
    bool save() const;
    int createPlayer();
    void signIn();
    void changeAddress();
    void removePlayer();
    void showSignedIn() const;

    std::string filename;
    std::vector<Player> players;
    std::string adminPin;
    int playerSession = 0;
    bool adminSession = false;
};

#endif
