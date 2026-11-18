# Dart Checker

TU Dublin COMP2603 C++ terminal project for 301, 501, clubs, leagues and four-player knockout tournaments.

## Share the project

Send your teammate pull request #1. Clone the review branch in PowerShell and open it in VS Code. [Pull request #1](https://github.com/hugomcglynn/Dart_Checker_Project/pull/1)

```powershell
git clone --branch codex/dart-checker-integration https://github.com/hugomcglynn/Dart_Checker_Project.git
cd Dart_Checker_Project
code .
```

Each checkout has its own local accounts and match history. Share the code through GitHub and keep the `dart_*.txt` saves private.

## Run on Windows in VS Code

1. In VS Code, choose **File → Open Folder** and select the folder that contains `Source`.
2. Choose **Terminal → New Terminal** and select PowerShell. Run `g++ --version` to check that MinGW's compiler is on PATH.
3. Run the commands below from the project folder. You can also use **Ctrl+Shift+B** and select **Build Dart Checker** to build the executable.

```powershell
$sources = (Get-ChildItem -File .\Source\*.cpp).FullName
g++ -std=c++17 -Wall -Wextra -pedantic -g $sources -o darts.exe -lbcrypt
.\darts.exe
```

Run `.\darts.exe --demo` for fictional players, automatically scored 301 and 501 games, a league table and a cup final with a prize. Each demo prints the location of its own temporary data folder.

The VS Code build task uses Windows CNG on Windows and links against OpenSSL's `libcrypto` on other platforms. Install the OpenSSL development headers and library before building on those platforms.

## Accounts and passwords

- Open **4. Player Profiles → 1. Sign in / set up admin → 1. Admin**. The person responsible for the local data creates and confirms the admin password on first use.
- The **Admin** can add or deactivate accounts and select any player in **2. View profile**, including an inactive player. The profile shows their name, age, nationality, address, and game statistics.
- A **Player** signs in with their player ID and password. They can see their own profile and edit their address. Signed-out users can see public names and match results.
- Choose **7. Change password** while signed in to update your password. New passwords are 8 to 128 bytes. Passwords are stored as salted hashes and omitted from all profile views.
- Existing account saves are upgraded on load. Enter your old PIN at the first sign-in, then choose and confirm a password. Player IDs and results carry over.

## Using the menu

- **New Game** accepts existing player IDs or lets you create a player. Enter one total each visit. A plain total counts as three darts unless the app asks for a dart count. Enter `81/2` to report a two-dart visit. On a checkout, confirm whether the last dart was a double. Enter `q` at a scoring prompt to abandon the match.
- **Leagues** schedules each pair once and awards two points per win. **Tournaments** advances both semifinal winners to the final and credits any prize to the champion. Set a tournament prize in whole euros while signed in as admin.
- **Leaderboards** orders active players by wins, then three-dart average, then player ID. Move compares rank immediately before and after the latest completed game. Bust darts count toward the average and earn zero points.

The app saves players, game history and competitions in three local `dart_*.txt` files in the terminal's working directory. Use one running copy per data folder. Profile fields are readable in the local player save by anyone with access to the file. Use fictional data for demos. Deactivated players remain named in old results.

## Ten-minute demo

| Time | Presenter | What to show |
| --- | --- | --- |
| 0–3 min | Gary | Add a fictional player as admin, sign out, and sign in as that player to change the address. |
| 3–6 min | Hugo | Start 301 with two players. Enter `180`, `0`, `81`, `0`, `40`, `n`, `2`, `0`, `40`, `y`, `1` to show a bust on 40 and then a D20 win. |
| 6–10 min | Brendan | Run `.\darts.exe --demo` and show the points table, the semifinal winners meeting in the final, and the champion's prize in the leaderboard. |

Hugo's menu and original scoring were on GitHub. Brendan supplied the clubs, leagues and tournament patch. Gary's assigned player and records area was implemented during integration.
