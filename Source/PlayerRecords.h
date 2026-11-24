// Gary is responsible for accounts and profiles in the team plan.
// Codex implemented this file during integration.

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
    bool isActive(int id) const;
    std::vector<int> activeIds() const;
    std::string name(int id) const;
    void listPlayers(bool includeInactive = false) const;
    int selectPlayer(bool allowCreate = false);
    int addPlayer(const std::string& name, int age, const std::string& nationality,
                  const std::string& address, const std::string& password);

    bool isAdmin() const;
    void accountMenu(const MatchHistory& history);

private:
    static const int ADMIN_SESSION = -1;

    bool save() const;
    int findIndex(int id) const;
    const Player* find(int id) const;
    int signedInPlayer() const;
    bool showProfile(int id) const;
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
    int signedInId = 0; // 0 is signed out. A positive value is a player ID.
};

#endif
