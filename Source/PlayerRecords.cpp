// Gary is responsible for accounts and profiles in the team plan.
// Codex implemented this file during integration.

#include "PlayerRecords.h"
#include "MatchHistory.h"
#include "Password.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

static string readText(const string& prompt)
{
    cout << prompt;
    string text;
    getline(cin, text);
    return text;
}

static int readNumber(const string& prompt)
{
    istringstream input(readText(prompt));
    int number;
    if (input >> number && (input >> ws).eof())
        return number;
    return numeric_limits<int>::min();
}

static bool validLegacyPin(const string& pin)
{
    return pin.size() >= 4 && pin.size() <= 12 &&
           pin.find_first_not_of("0123456789") == string::npos;
}

static bool validNewPassword(const string& password)
{
    return password.size() >= 8 && password.size() <= 128 &&
           password.find_first_not_of(" \t") != string::npos;
}

static string readNewPassword()
{
    string password = readPassword("New password (8-128 bytes, blank to cancel): ");
    if (password.empty())
        return "";
    if (!validNewPassword(password))
    {
        cout << "Choose a password of 8 to 128 bytes with at least one non-space character.\n";
        return "";
    }
    if (readPassword("Confirm password: ") != password)
    {
        cout << "Passwords did not match.\n";
        return "";
    }
    return password;
}

PlayerRecords::PlayerRecords(const string& filename) : filename(filename)
{
}

bool PlayerRecords::load()
{
    ifstream input(filename);
    if (!input)
    {
        error_code error;
        return !filesystem::exists(filename, error) && !error;
    }

    string version, savedAdminHash;
    if (!(input >> version >> quoted(savedAdminHash)) ||
        (version != "DART_PLAYERS_1" && version != "DART_PLAYERS_2"))
        return false;

    const bool legacy = version == "DART_PLAYERS_1";
    int adminUpgrade = 0;
    if (legacy && !savedAdminHash.empty())
    {
        if (!validLegacyPin(savedAdminHash))
            return false;
        savedAdminHash = hashPassword(savedAdminHash);
        if (savedAdminHash.empty())
            return false;
        adminUpgrade = 1;
    }
    if (!legacy && (!(input >> adminUpgrade) || (adminUpgrade != 0 && adminUpgrade != 1) ||
                    (savedAdminHash.empty() ? adminUpgrade != 0 : !validPasswordHash(savedAdminHash))))
        return false;

    vector<Player> savedPlayers;
    input >> ws;
    while (!input.eof())
    {
        int id, age, active;
        int upgrade = 0;
        string name, nationality, address, hash;
        if (!(input >> id >> active >> age >> quoted(name) >> quoted(nationality) >>
              quoted(address) >> quoted(hash)))
            return false;

        if (id < 1 || (!savedPlayers.empty() && id <= savedPlayers.back().getId()) ||
            (active != 0 && active != 1) || age < 1 || age > 120 ||
            name.find_first_not_of(" \t\r\n") == string::npos)
            return false;

        if (legacy)
        {
            if (!validLegacyPin(hash))
                return false;
            hash = hashPassword(hash);
            if (hash.empty())
                return false;
            upgrade = 1;
        }
        else if (!(input >> upgrade) || (upgrade != 0 && upgrade != 1) || !validPasswordHash(hash))
            return false;

        savedPlayers.emplace_back(id, name, age, nationality, address, hash, active == 1, upgrade == 1);
        input >> ws;
    }
    if (input.bad())
        return false;

    input.close();
    adminHash = savedAdminHash;
    adminNeedsNewPassword = adminUpgrade == 1;
    players.swap(savedPlayers);
    accountRole = AccountRole::Guest;
    playerSession = 0;
    // Replace a PIN-era file as soon as it is read so it no longer stores plaintext PINs.
    return !legacy || save();
}

bool PlayerRecords::save() const
{
    const string temporary = filename + ".tmp";
    ofstream output(temporary);
    if (!output)
        return false;

    output << "DART_PLAYERS_2 " << quoted(adminHash) << ' ' << adminNeedsNewPassword << '\n';
    for (const Player& player : players)
    {
        output << player.id << ' ' << player.active << ' ' << player.age << ' '
               << quoted(player.name) << ' ' << quoted(player.nationality) << ' '
               << quoted(player.address) << ' ' << quoted(player.passwordHash)
               << ' ' << player.needsNewPassword << '\n';
    }
    output.close();
    if (!output)
        return false;

    // Replace the old file after its replacement has been written successfully.
    error_code error;
    filesystem::rename(temporary, filename, error);
    if (!error)
        return true;

    filesystem::remove(temporary, error);
    return false;
}

int PlayerRecords::findIndex(int id) const
{
    for (int i = 0; i < static_cast<int>(players.size()); ++i)
    {
        if (players[i].id == id)
            return i;
    }
    return -1;
}

const Player* PlayerRecords::find(int id) const
{
    int index = findIndex(id);
    return index < 0 ? nullptr : &players[index];
}

bool PlayerRecords::isActive(int id) const
{
    const Player* player = find(id);
    return player != nullptr && player->active;
}

int PlayerRecords::lastId() const
{
    return players.empty() ? 0 : players.back().id;
}

string PlayerRecords::name(int id) const
{
    const Player* player = find(id);
    return player == nullptr ? "Player #" + to_string(id) : player->name;
}

void PlayerRecords::listPlayers() const
{
    bool found = false;
    for (const Player& player : players)
    {
        if (player.active)
        {
            cout << player.id << ". " << player.name << '\n';
            found = true;
        }
    }
    if (!found)
        cout << "No active players.\n";
}

void PlayerRecords::listProfiles() const
{
    for (const Player& player : players)
    {
        cout << player.id << ". " << player.name;
        if (!player.active)
            cout << " (inactive)";
        cout << '\n';
    }
    if (players.empty())
        cout << "No player profiles.\n";
}

int PlayerRecords::addPlayer(const string& name, int age, const string& nationality,
                             const string& address, const string& password)
{
    if (name.find_first_not_of(" \t\r\n") == string::npos || age < 1 || age > 120 ||
        !validNewPassword(password) || lastId() == numeric_limits<int>::max() ||
        name.find_first_of("\r\n") != string::npos ||
        nationality.find_first_of("\r\n") != string::npos ||
        address.find_first_of("\r\n") != string::npos)
        return 0;

    string hash = hashPassword(password);
    if (hash.empty())
        return 0;
    int id = lastId() + 1;
    players.emplace_back(id, name, age, nationality, address, hash);
    if (save())
        return id;

    players.pop_back();
    return 0;
}

int PlayerRecords::createPlayer()
{
    const string name = readText("Name (blank to cancel): ");
    if (name.find_first_not_of(" \t") == string::npos)
        return 0;

    const int age = readNumber("Age (1-120): ");
    if (age < 1 || age > 120)
    {
        cout << "Enter an age between 1 and 120.\n";
        return 0;
    }
    const string nationality = readText("Nationality: ");
    const string address = readText("Address: ");
    const string password = readNewPassword();
    if (password.empty())
        return 0;

    const int id = addPlayer(name, age, nationality, address, password);
    if (id != 0)
        cout << "Created " << name << " with player ID " << id << ".\n";
    else
        cout << "The player could not be saved.\n";
    return id;
}

int PlayerRecords::selectPlayer(bool allowCreate)
{
    while (cin)
    {
        listPlayers();
        const int id = readNumber(allowCreate ? "Player ID (-1 create player, 0 back): "
                                               : "Player ID (0 back): ");
        if (id == 0 || !cin)
            return 0;
        if (id == -1 && allowCreate)
            return createPlayer();
        if (isActive(id))
            return id;
        cout << "Select an active player ID.\n";
    }
    return 0;
}

bool PlayerRecords::isAdmin() const
{
    return accountRole == AccountRole::Admin;
}

int PlayerRecords::signedInPlayer() const
{
    return playerSession;
}

bool PlayerRecords::replacePassword(string& storedHash, bool& needsNewPassword)
{
    string password = readNewPassword();
    if (password.empty())
        return false;

    string hash = hashPassword(password);
    if (hash.empty())
    {
        cout << "The password could not be secured.\n";
        return false;
    }

    string previousHash = storedHash;
    bool previousUpgrade = needsNewPassword;
    storedHash = hash;
    needsNewPassword = false;
    if (save())
    {
        cout << "Password saved.\n";
        return true;
    }

    storedHash = previousHash;
    needsNewPassword = previousUpgrade;
    cout << "The password could not be saved.\n";
    return false;
}

bool PlayerRecords::authenticate(string& storedHash, bool& needsNewPassword, const string& label)
{
    string password = readPassword(label + (needsNewPassword ? " previous PIN: " : " password: "));
    if (!verifyPassword(password, storedHash))
    {
        cout << label << " sign-in failed.\n";
        return false;
    }
    if (needsNewPassword)
    {
        cout << "Choose a new password to finish signing in.\n";
        return replacePassword(storedHash, needsNewPassword);
    }
    return true;
}

void PlayerRecords::signIn()
{
    const int role = readNumber("Sign in as 1. Admin, 2. Player, 0. Back: ");
    if (role == 1 || role == 2)
    {
        accountRole = AccountRole::Guest;
        playerSession = 0;
    }

    if (role == 1)
    {
        if (adminHash.empty())
        {
            cout << "Set up the administrator account for this data folder.\n";
            if (!replacePassword(adminHash, adminNeedsNewPassword))
                return;
        }
        else if (!authenticate(adminHash, adminNeedsNewPassword, "Admin"))
            return;

        accountRole = AccountRole::Admin;
    }
    else if (role == 2)
    {
        const int id = selectPlayer();
        if (id == 0)
            return;
        Player& player = players[findIndex(id)];
        if (!authenticate(player.passwordHash, player.needsNewPassword, "Player"))
            return;
        accountRole = AccountRole::Player;
        playerSession = id;
    }
    else if (role != 0 && cin)
    {
        cout << "Invalid sign-in option.\n";
    }
}

void PlayerRecords::changePassword()
{
    if (isAdmin())
    {
        if (authenticate(adminHash, adminNeedsNewPassword, "Admin"))
            replacePassword(adminHash, adminNeedsNewPassword);
        return;
    }
    if (accountRole == AccountRole::Player)
    {
        Player& player = players[findIndex(playerSession)];
        if (authenticate(player.passwordHash, player.needsNewPassword, "Player"))
            replacePassword(player.passwordHash, player.needsNewPassword);
        return;
    }
    cout << "Sign in to change your password.\n";
}

bool PlayerRecords::showProfile(int id) const
{
    if (!isAdmin() && (accountRole != AccountRole::Player || playerSession != id))
    {
        cout << "Only the player or admin can view these details.\n";
        return false;
    }

    const Player* player = find(id);
    if (player == nullptr)
    {
        cout << "Player ID not found.\n";
        return false;
    }
    cout << "\nPlayer ID: " << player->id << "\nName: " << player->name
         << "\nAccount type: Player"
         << "\nAge: " << player->age << "\nNationality: " << player->nationality
         << "\nAddress: " << player->address << '\n';
    if (!player->active)
        cout << "Player no longer active.\n";
    return true;
}

void PlayerRecords::changeAddress()
{
    if (playerSession == 0)
    {
        cout << "Sign in as a player to update your address.\n";
        return;
    }

    const string address = readText("New address (blank to cancel): ");
    if (address.find_first_not_of(" \t") == string::npos)
        return;

    for (Player& player : players)
    {
        if (player.id == playerSession)
        {
            const string oldAddress = player.address;
            player.address = address;
            if (save())
                cout << "Address updated.\n";
            else
            {
                player.address = oldAddress;
                cout << "The address could not be saved.\n";
            }
            return;
        }
    }
}

void PlayerRecords::removePlayer()
{
    const int id = selectPlayer();
    if (id == 0)
        return;

    for (Player& player : players)
    {
        if (player.id == id)
        {
            // Keep the ID and name available for past games.
            player.active = false;
            if (save())
                cout << "Removed " << player.name << " from active players.\n";
            else
            {
                player.active = true;
                cout << "The change could not be saved.\n";
            }
            return;
        }
    }
}

void PlayerRecords::showSignedIn() const
{
    if (isAdmin())
        cout << "\nSigned in as admin.\n";
    else if (playerSession != 0)
        cout << "\nSigned in as " << name(playerSession) << ".\n";
    else
        cout << "\nSigned out.\n";
}

void PlayerRecords::accountMenu(const MatchHistory& history)
{
    while (cin)
    {
        showSignedIn();
        cout << "1. Sign in / set up admin\n2. View profile\n3. Change my address\n"
             << "4. Add player (admin)\n5. Remove player (admin)\n6. Sign out\n"
             << "7. Change password\n0. Back\n";
        const int choice = readNumber("Enter choice: ");
        if (!cin || choice == 0)
            return;

        switch (choice)
        {
            case 1:
                signIn();
                break;
            case 2:
            {
                int id = playerSession;
                if (isAdmin())
                {
                    listProfiles();
                    id = readNumber("Player ID (0 back): ");
                    if (id == 0 || !cin)
                        break;
                }
                if (showProfile(id))
                    history.showPlayerStats(id);
                break;
            }
            case 3:
                changeAddress();
                break;
            case 4:
                if (isAdmin())
                    createPlayer();
                else
                    cout << "Sign in as admin to add a player here.\n";
                break;
            case 5:
                if (isAdmin())
                    removePlayer();
                else
                    cout << "Sign in as admin to remove players.\n";
                break;
            case 6:
                accountRole = AccountRole::Guest;
                playerSession = 0;
                break;
            case 7:
                changePassword();
                break;
            default:
                cout << "Invalid selection.\n";
        }
    }
}
