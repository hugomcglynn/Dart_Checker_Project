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
    char extra;
    if (input >> number && !(input >> extra))
        return number;
    return numeric_limits<int>::min();
}

static bool validLegacyPin(const string& pin)
{
    return pin.size() >= 4 && pin.size() <= 12 &&
           pin.find_first_not_of("0123456789") == string::npos;
}

// Version 1 contains PINs. Version 2 contains hashes and a PIN-upgrade flag.
static bool readSavedPassword(istream& input, bool legacy, string& hash,
                              bool& needsNewPassword, bool allowEmpty = false)
{
    if (!(input >> quoted(hash)))
        return false;

    if (legacy)
    {
        if (hash.empty())
            return allowEmpty;
        if (!validLegacyPin(hash))
            return false;
        hash = hashPassword(hash);
        needsNewPassword = true;
        return !hash.empty();
    }

    int upgrade;
    if (!(input >> upgrade) || (upgrade != 0 && upgrade != 1))
        return false;
    needsNewPassword = upgrade == 1;
    if (hash.empty())
        return allowEmpty && !needsNewPassword;
    return validPasswordHash(hash);
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

    string version;
    if (!(input >> version) || (version != "DART_PLAYERS_1" && version != "DART_PLAYERS_2"))
        return false;

    const bool legacy = version == "DART_PLAYERS_1";
    string savedAdminHash;
    bool adminUpgrade = false;
    if (!readSavedPassword(input, legacy, savedAdminHash, adminUpgrade, true))
        return false;

    vector<Player> savedPlayers;
    input >> ws;
    while (!input.eof())
    {
        int id, age, active;
        bool upgrade = false;
        string name, nationality, address, hash;
        if (!(input >> id >> active >> age >> quoted(name) >> quoted(nationality) >> quoted(address)))
            return false;

        if (id < 1 || (!savedPlayers.empty() && id <= savedPlayers.back().getId()) ||
            (active != 0 && active != 1) || age < 1 || age > 120 ||
            name.find_first_not_of(" \t\r\n") == string::npos)
            return false;

        if (!readSavedPassword(input, legacy, hash, upgrade))
            return false;

        savedPlayers.emplace_back(id, name, age, nationality, address, hash, active == 1, upgrade);
        input >> ws;
    }
    if (input.bad())
        return false;

    input.close();
    adminHash = savedAdminHash;
    adminNeedsNewPassword = adminUpgrade;
    players.swap(savedPlayers);
    signedInId = 0;
    // Save the hashes immediately to remove the original PINs.
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
    return player != nullptr && player->isActive();
}

vector<int> PlayerRecords::activeIds() const
{
    vector<int> ids;
    for (const Player& player : players)
    {
        if (player.isActive())
            ids.push_back(player.getId());
    }
    return ids;
}

string PlayerRecords::name(int id) const
{
    const Player* player = find(id);
    return player == nullptr ? "Player #" + to_string(id) : player->getName();
}

void PlayerRecords::listPlayers(bool includeInactive) const
{
    bool found = false;
    for (const Player& player : players)
    {
        if (!player.active && !includeInactive)
            continue;
        cout << player.id << ". " << player.name;
        if (!player.active)
            cout << " (inactive)";
        cout << '\n';
        found = true;
    }
    if (!found)
        cout << (includeInactive ? "No player profiles.\n" : "No active players.\n");
}

int PlayerRecords::addPlayer(const string& name, int age, const string& nationality,
                             const string& address, const string& password)
{
    int lastId = players.empty() ? 0 : players.back().getId();
    if (name.find_first_not_of(" \t\r\n") == string::npos || age < 1 || age > 120 ||
        !validNewPassword(password) || lastId == numeric_limits<int>::max() ||
        name.find_first_of("\r\n") != string::npos ||
        nationality.find_first_of("\r\n") != string::npos ||
        address.find_first_of("\r\n") != string::npos)
        return 0;

    string hash = hashPassword(password);
    if (hash.empty())
        return 0;
    int id = lastId + 1;
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
    return signedInId == ADMIN_SESSION;
}

int PlayerRecords::signedInPlayer() const
{
    return signedInId > 0 ? signedInId : 0;
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
        signedInId = 0;

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

        signedInId = ADMIN_SESSION;
    }
    else if (role == 2)
    {
        const int id = selectPlayer();
        if (id == 0)
            return;
        Player& player = players[findIndex(id)];
        if (!authenticate(player.passwordHash, player.needsNewPassword, "Player"))
            return;
        signedInId = id;
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
    if (signedInPlayer() != 0)
    {
        Player& player = players[findIndex(signedInId)];
        if (authenticate(player.passwordHash, player.needsNewPassword, "Player"))
            replacePassword(player.passwordHash, player.needsNewPassword);
        return;
    }
    cout << "Sign in to change your password.\n";
}

bool PlayerRecords::showProfile(int id) const
{
    if (!isAdmin() && (signedInPlayer() == 0 || signedInId != id))
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
    if (signedInPlayer() == 0)
    {
        cout << "Sign in as a player to update your address.\n";
        return;
    }

    const string address = readText("New address (blank to cancel): ");
    if (address.find_first_not_of(" \t") == string::npos)
        return;

    Player& player = players[findIndex(signedInId)];
    const string oldAddress = player.address;
    player.address = address;
    if (save())
        cout << "Address updated.\n";
    else
    {
        player.address = oldAddress;
        cout << "The address could not be saved.\n";
    }
}

void PlayerRecords::removePlayer()
{
    const int id = selectPlayer();
    if (id == 0)
        return;

    // Keep the ID and name for past games.
    Player& player = players[findIndex(id)];
    player.active = false;
    if (save())
        cout << "Removed " << player.name << " from active players.\n";
    else
    {
        player.active = true;
        cout << "The change could not be saved.\n";
    }
}

void PlayerRecords::showSignedIn() const
{
    if (isAdmin())
        cout << "\nSigned in as admin.\n";
    else if (signedInId != 0)
        cout << "\nSigned in as " << name(signedInId) << ".\n";
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
                int id = signedInPlayer();
                if (isAdmin())
                {
                    listPlayers(true);
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
                signedInId = 0;
                break;
            case 7:
                changePassword();
                break;
            default:
                cout << "Invalid selection.\n";
        }
    }
}
