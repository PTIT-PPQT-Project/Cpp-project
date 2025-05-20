// include/utils/HashUtils.hpp
#pragma once

#include <string>

class HashUtils {
public:
    HashUtils(); // Constructor to print security warning

    std::string generateUUID() const;
    std::string generateSalt(size_t length = 16) const;

    // DEMONSTRATION HASH - NOT SECURE
    std::string hashPassword(const std::string& password, const std::string& salt) const;
    bool verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt) const;
    
    std::string generateRandomPassword(size_t length = 12) const;
};