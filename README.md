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

Before presenting, build the app and create two fictional player accounts. Have their player IDs ready for Hugo. In a second terminal, run `.\darts.exe --demo` and leave the output ready for Brendan.

| Time | Presenter | What to show |
| --- | --- | --- |
| 0:00–3:00 | Gary | **Accounts and privacy.** Show that an admin can select any player profile. Sign out, sign in as a fictional player, and show that player's own profile. |
| 3:00–7:00 | Hugo | **Live scoring.** Play a 301 match with two prepared players. Explain a bust and a double finish, then show the saved match in Game History. Mention that 501 uses the same rules. |
| 7:00–10:00 | Brendan | **Prepared results.** Switch to the demo output. Point out the league table and the cup final winner's €50 prize. Finish within three minutes. |

Hugo can enter these values one at a time when prompted in the 301 match. The first player reaches 40, busts with two single 20s, then wins with double 20.

```text
180, 0, 81, 0, 40, n, 2, 0, 40, y, 1
```

Brendan can say, “Every pair plays once in the league, and a win earns two points. In the cup, the two semifinal winners meet in the final. Here’s the champion and the €50 prize.”

## Source credits

Each C++ source and header has a credit at the top. The credits distinguish the original code, the team's assigned areas and the work Codex completed during integration.

| Team member | Files |
| --- | --- |
| Hugo McGlynn | Wrote the original `menu.cpp`, `menu.h`, `score.cpp`, `score.h` and the starter `main.cpp` in the repository root. Codex wrote `Game.cpp` and `Game.h` in Hugo's gameplay area using his scoring rules. |
| Brendan Galvin | Supplied `Club.cpp`, `Club.h`, `Competitions.cpp`, `Competitions.h`, `League.cpp`, `League.h`, `Tournament.cpp`, `Tournament.h` and the first working `Source/main.cpp` in his patch. His patch also updated the menu and corrected the scoring include. |
| Gary | Assigned player accounts and records. Codex implemented `Player.cpp`, `Player.h`, `PlayerRecords.cpp`, `PlayerRecords.h`, `MatchHistory.cpp`, `MatchHistory.h`, `Password.cpp` and `Password.h` in that area. |
