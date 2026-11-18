// This file supports Gary's player accounts and records area.
// Codex implemented this file during integration.

#ifndef PASSWORD_H
#define PASSWORD_H

#include <string>

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& password, const std::string& stored);
bool validPasswordHash(const std::string& stored);
std::string readPassword(const std::string& prompt);

#endif
