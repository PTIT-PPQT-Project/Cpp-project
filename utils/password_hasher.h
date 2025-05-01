#pragma once
#include <string>
#include <functional>
#include <random>
#include <iomanip>
#include <sstream>

class PasswordHasher {
private:
    static std::string hashToHex(size_t hashValue);
    static std::string generateSalt(size_t length = 16);
    static std::string computeHash(const std::string& input);
    static std::string hashWithSalt(const std::string& password, const std::string& salt, int iterations = 10000);
    
public:
    static std::pair<std::string, std::string> hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
}; 