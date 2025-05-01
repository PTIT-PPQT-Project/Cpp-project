#include "password_hasher.h"
#include <random>
#include <iomanip>
#include <sstream>

std::string PasswordHasher::hashToHex(size_t hashValue) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hashValue;
    return ss.str();
}

std::string PasswordHasher::generateSalt(size_t length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, sizeof(alphanum) - 2);
    
    std::string salt;
    salt.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        salt += alphanum[distrib(gen)];
    }
    
    return salt;
}

std::string PasswordHasher::computeHash(const std::string& input) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(input);
    return hashToHex(hashValue);
}

std::string PasswordHasher::hashWithSalt(const std::string& password, const std::string& salt, int iterations) {
    std::string combined = password + salt;
    
    std::string hash = computeHash(combined);
    
    for (int i = 0; i < iterations; ++i) {
        hash = computeHash(hash + salt);
    }
    
    return hash;
}

std::pair<std::string, std::string> PasswordHasher::hashPassword(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashWithSalt(password, salt);
    return {hash, salt};
}

bool PasswordHasher::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string computedHash = hashWithSalt(password, salt);
    return computedHash == hash;
} 