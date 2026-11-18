// Brendan Galvin supplied the first working app entry point.
// Codex added command-line and demo support.

#include "menu.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc == 1)
        return mainMenu();
    if (argc == 2 && std::string(argv[1]) == "--demo")
        return demo();
    std::cerr << "Usage: darts [--demo]\n";
    return 1;
}
