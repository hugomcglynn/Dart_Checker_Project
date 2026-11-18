#include "PlayerRecords.h"
#include "MatchHistory.h"
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

static bool validPin(const string& pin)
{
    return pin.size() >= 4 && pin.size() <= 12 &&
           pin.find_first_not_of("0123456789") == string::npos;
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

    string version, savedAdminPin;
    if (!(input >> version >> quoted(savedAdminPin)) || version != "DART_PLAYERS_1" ||
        (!savedAdminPin.empty() && !validPin(savedAdminPin)))
        return false;

    vector<Player> savedPlayers;
    input >> ws;
    while (!input.eof())
    {
        int id, age, active;
        string name, nationality, address, pin;
        if (!(input >> id >> active >> age >> quoted(name) >> quoted(nationality) >>
              quoted(address) >> quoted(pin)))
            return false;

        if (id < 1 || (!savedPlayers.empty() && id <= savedPlayers.back().getId()) ||
            (active != 0 && active != 1) || age < 1 || age > 120 ||
            name.find_first_not_of(" \t\r\n") == string::npos || !validPin(pin))
            return false;

        savedPlayers.emplace_back(id, name, age, nationality, address, pin, active == 1);
        input >> ws;
    }
    if (input.bad())
        return false;

    adminPin = savedAdminPin;
    players.swap(savedPlayers);
    adminSession = false;
    playerSession = 0;
    return true;
}

bool PlayerRecords::save() const
{
    const string temporary = filename + ".tmp";
    ofstream output(temporary);
    if (!output)
        return false;

    output << "DART_PLAYERS_1 " << quoted(adminPin) << '\n';
    for (const Player& player : players)
    {
        output << player.id << ' ' << player.active << ' ' << player.age << ' '
               << quoted(player.name) << ' ' << quoted(player.nationality) << ' '
               << quoted(player.address) << ' ' << quoted(player.pin) << '\n';
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

const Player* PlayerRecords::find(int id) const
{
    for (const Player& player : players)
    {
        if (player.id == id)
            return &player;
    }
    return nullptr;
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

int PlayerRecords::addPlayer(const string& name, int age, const string& nationality,
                             const string& address, const string& pin)
{
    if (name.find_first_not_of(" \t\r\n") == string::npos || age < 1 || age > 120 ||
        !validPin(pin) || lastId() == numeric_limits<int>::max() ||
        name.find_first_of("\r\n") != string::npos ||
        nationality.find_first_of("\r\n") != string::npos ||
        address.find_first_of("\r\n") != string::npos)
        return 0;

    int id = lastId() + 1;
    players.emplace_back(id, name, age, nationality, address, pin);
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
    const string pin = readText("Player PIN (4-12 digits): ");
    if (!validPin(pin))
    {
        cout << "Enter a PIN with 4 to 12 digits.\n";
        return 0;
    }

    const int id = addPlayer(name, age, nationality, address, pin);
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
    return adminSession;
}

int PlayerRecords::signedInPlayer() const
{
    return playerSession;
}

void PlayerRecords::signIn()
{
    const int role = readNumber("Sign in as 1. Admin, 2. Player, 0. Back: ");
    if (role == 1)
    {
        const bool firstLogin = adminPin.empty();
        const string pin = readText(firstLogin ? "Create an admin PIN (4-12 digits): "
                                               : "Admin PIN: ");
        if ((firstLogin && !validPin(pin)) || (!firstLogin && pin != adminPin))
        {
            cout << "Admin sign-in failed.\n";
            return;
        }

        if (firstLogin)
        {
            adminPin = pin;
            if (!save())
            {
                adminPin.clear();
                cout << "The admin PIN could not be saved.\n";
                return;
            }
        }
        adminSession = true;
        playerSession = 0;
    }
    else if (role == 2)
    {
        const int id = selectPlayer();
        if (id == 0)
            return;
        const Player* player = find(id);
        if (readText("Player PIN: ") != player->pin)
        {
            cout << "Player sign-in failed.\n";
            return;
        }
        adminSession = false;
        playerSession = id;
    }
    else if (role != 0 && cin)
    {
        cout << "Invalid sign-in option.\n";
    }
}

bool PlayerRecords::showProfile(int id) const
{
    if (!adminSession && (playerSession == 0 || playerSession != id))
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
    if (adminSession)
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
             << "4. Add player (admin)\n5. Remove player (admin)\n6. Sign out\n0. Back\n";
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
                if (adminSession)
                {
                    listPlayers();
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
                if (adminSession)
                    createPlayer();
                else
                    cout << "Sign in as admin to add a player here.\n";
                break;
            case 5:
                if (adminSession)
                    removePlayer();
                else
                    cout << "Sign in as admin to remove players.\n";
                break;
            case 6:
                adminSession = false;
                playerSession = 0;
                break;
            default:
                cout << "Invalid selection.\n";
        }
    }
}
