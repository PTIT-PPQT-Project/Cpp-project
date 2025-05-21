// include/utils/HashUtils.hpp
#pragma once

#include <string>
#include <vector> // Required by some services, but not directly by HashUtils in this basic form

class HashUtils {
public:
    HashUtils();

    // Generates a pseudo-UUID (for demonstration; not a standard UUID)
    std::string generateUUID() const;

    // Generates a random salt string
    std::string generateSalt(size_t length = 16) const;

    // Hashes a password with a given salt (DEMONSTRATION HASH - NOT SECURE)
    std::string hashPassword(const std::string& password, const std::string& salt) const;

    // Verifies a password against a stored hash and salt (DEMONSTRATION - NOT SECURE)
    bool verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt) const;

    // Generates a random password
    std::string generateRandomPassword(size_t length = 12) const;
};