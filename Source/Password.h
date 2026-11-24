// Gary's accounts area. Implemented by Codex.

#ifndef PASSWORD_H
#define PASSWORD_H

#include <string>

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& password, const std::string& stored);
bool validPasswordHash(const std::string& stored);
std::string readPassword(const std::string& prompt);

#endif
