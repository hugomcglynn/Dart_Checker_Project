#include "Password.h"

#include <array>
#include <iostream>
#include <limits>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace
{
constexpr int iterations = 600000;
const std::string hashPrefix = "pbkdf2-sha256$" + std::to_string(iterations) + "$";
using Salt = std::array<unsigned char, 16>;
using Key = std::array<unsigned char, 32>;

template <std::size_t Count>
std::string encodeHex(const std::array<unsigned char, Count>& bytes)
{
    const char digits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(2 * Count);
    for (unsigned char byte : bytes)
    {
        hex += digits[byte >> 4];
        hex += digits[byte & 15];
    }
    return hex;
}

int hexDigit(char digit)
{
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    return -1;
}

template <std::size_t Count>
bool decodeHex(std::string_view hex, std::array<unsigned char, Count>& bytes)
{
    if (hex.size() != 2 * Count)
        return false;
    for (std::size_t i = 0; i < Count; ++i)
    {
        int high = hexDigit(hex[2 * i]);
        int low = hexDigit(hex[2 * i + 1]);
        if (high < 0 || low < 0)
            return false;
        bytes[i] = static_cast<unsigned char>(16 * high + low);
    }
    return true;
}

bool decodeHash(std::string_view stored, Salt& salt, Key& key)
{
    const std::size_t separator = hashPrefix.size() + 2 * salt.size();
    return stored.size() == separator + 1 + 2 * key.size()
        && stored.substr(0, hashPrefix.size()) == hashPrefix
        && stored[separator] == '$'
        && decodeHex(stored.substr(hashPrefix.size(), 2 * salt.size()), salt)
        && decodeHex(stored.substr(separator + 1), key);
}

bool generateSalt(Salt& salt)
{
#ifdef _WIN32
    return BCryptGenRandom(nullptr, salt.data(), static_cast<ULONG>(salt.size()),
                            BCRYPT_USE_SYSTEM_PREFERRED_RNG) >= 0;
#else
    return RAND_bytes(salt.data(), static_cast<int>(salt.size())) == 1;
#endif
}

bool deriveKey(const std::string& password, Salt& salt, Key& key)
{
    if (password.empty() || password.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return false;

#ifdef _WIN32
    BCRYPT_ALG_HANDLE provider;
    if (BCryptOpenAlgorithmProvider(&provider, BCRYPT_SHA256_ALGORITHM, nullptr,
                                    BCRYPT_ALG_HANDLE_HMAC_FLAG) < 0)
        return false;

    // CNG's password argument is an input buffer despite its mutable pointer type.
    auto* passwordBytes = reinterpret_cast<PUCHAR>(const_cast<char*>(password.data()));
    NTSTATUS status = BCryptDeriveKeyPBKDF2(provider, passwordBytes,
        static_cast<ULONG>(password.size()), salt.data(), static_cast<ULONG>(salt.size()),
        iterations, key.data(), static_cast<ULONG>(key.size()), 0);
    BCryptCloseAlgorithmProvider(provider, 0);
    return status >= 0;
#else
    return PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
        salt.data(), static_cast<int>(salt.size()), iterations, EVP_sha256(),
        static_cast<int>(key.size()), key.data()) == 1;
#endif
}

bool keysMatch(const Key& expected, const Key& actual)
{
#ifdef _WIN32
    // Read every byte so verification has no early exit at a mismatch.
    volatile unsigned int difference = 0;
    for (std::size_t i = 0; i < expected.size(); ++i)
        difference |= expected[i] ^ actual[i];
    return difference == 0;
#else
    return CRYPTO_memcmp(expected.data(), actual.data(), expected.size()) == 0;
#endif
}

class HiddenInput
{
public:
    HiddenInput()
    {
#ifdef _WIN32
        hidden = GetConsoleMode(terminal, &original)
            && SetConsoleMode(terminal, original & ~ENABLE_ECHO_INPUT);
#else
        if (tcgetattr(STDIN_FILENO, &original) == 0)
        {
            termios mode = original;
            mode.c_lflag &= ~(ECHO | ECHONL);
            hidden = tcsetattr(STDIN_FILENO, TCSANOW, &mode) == 0;
        }
#endif
    }

    ~HiddenInput()
    {
        if (!hidden)
            return;
#ifdef _WIN32
        SetConsoleMode(terminal, original);
#else
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
#endif
    }

    bool isHidden() const { return hidden; }

private:
    bool hidden = false;
#ifdef _WIN32
    HANDLE terminal = GetStdHandle(STD_INPUT_HANDLE);
    DWORD original = 0;
#else
    termios original{};
#endif
};
}

std::string hashPassword(const std::string& password)
{
    Salt salt{};
    Key key{};
    if (!generateSalt(salt) || !deriveKey(password, salt, key))
        return {};
    return hashPrefix + encodeHex(salt) + "$" + encodeHex(key);
}

bool verifyPassword(const std::string& password, const std::string& stored)
{
    Salt salt{};
    Key expected{};
    Key actual{};
    return decodeHash(stored, salt, expected) && deriveKey(password, salt, actual)
        && keysMatch(expected, actual);
}

bool validPasswordHash(const std::string& stored)
{
    Salt salt{};
    Key key{};
    return decodeHash(stored, salt, key);
}

std::string readPassword(const std::string& prompt)
{
    HiddenInput input;
    std::cout << prompt << std::flush;
    std::string password;
    std::getline(std::cin, password);
    if (input.isHidden())
        std::cout << '\n';
    return password;
}
