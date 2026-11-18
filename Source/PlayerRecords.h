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
                  const std::string& address, const std::string& password);

    bool isAdmin() const;
    int signedInPlayer() const;
    void accountMenu(const MatchHistory& history);
    bool showProfile(int id) const;

private:
    enum class AccountRole { Guest, Player, Admin };

    bool save() const;
    int findIndex(int id) const;
    void listProfiles() const;
    int createPlayer();
    void signIn();
    bool authenticate(std::string& storedHash, bool& needsNewPassword, const std::string& label);
    bool replacePassword(std::string& storedHash, bool& needsNewPassword);
    void changePassword();
    void changeAddress();
    void removePlayer();
    void showSignedIn() const;

    std::string filename;
    std::vector<Player> players;
    std::string adminHash;
    bool adminNeedsNewPassword = false;
    int playerSession = 0;
    AccountRole accountRole = AccountRole::Guest;
};

#endif
