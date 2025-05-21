// src/utils/HashUtils.cpp
#include "../include/utils/HashUtils.hpp"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip> // For std::hex
#include <algorithm> // For std::generate_n
#include <iostream> // For security warning
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

HashUtils::HashUtils() {
    // Constructor
    std::cerr << "**********************************************************************" << std::endl;
    std::cerr << "WARNING: HashUtils is using DEMONSTRATION hashing functions." << std::endl;
    std::cerr << "These are NOT secure and MUST NOT be used in production." << std::endl;
    std::cerr << "Use a proper cryptographic library for password hashing." << std::endl;
    std::cerr << "**********************************************************************" << std::endl;
}

std::string HashUtils::generateUUID() const {
    // Simple pseudo-UUID generator (timestamp + random number)
    // NOT a standard UUID. For a real UUID, use a library.
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<unsigned long long> dist(0, 0xFFFFFFFFFFFFFFFFULL);

    long long time_now = std::chrono::system_clock::now().time_since_epoch().count();

    std::stringstream ss;
    ss << std::hex << (time_now & 0xFFFFFFFF) // Lower 32 bits of time
       << '-' << ((dist(generator) >> 32) & 0xFFFF) // Random 16 bits
       << '-' << ((dist(generator) >> 16) & 0xFFFF) // Random 16 bits
       << '-' << (dist(generator) & 0xFFFF)         // Random 16 bits
       << '-' << (dist(generator) & 0xFFFFFFFFFFFFULL); // Random 48 bits
    return ss.str();
}

std::string HashUtils::generateSalt(size_t length) const {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
    std::string random_string;
    random_string.reserve(length);
    std::generate_n(std::back_inserter(random_string), length,
                    [&]() { return CHARACTERS[distribution(generator)]; });
    return random_string;
}

// --- DEMONSTRATION HASHING - NOT SECURE ---
std::string HashUtils::hashPassword(const std::string& password, const std::string& salt) const {
    // This is a very basic, insecure hash for demonstration.
    // A real implementation would use something like SHA256 (from a crypto library)
    // combined with many iterations (e.g., PBKDF2) or a password-specific hash like bcrypt/Argon2.
    std::string saltedPassword = salt + password + salt; // Simple salting
    unsigned long hash = 5381; // djb2 seed
    for (char c : saltedPassword) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    std::stringstream ss;
    ss << std::hex << hash;
    return "demo_hash$" + ss.str(); // Prefix to indicate it's a demo hash
}

bool HashUtils::verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt) const {
    // Verify against the demonstration hash
    return hashedPassword == hashPassword(password, salt);
}
// --- END OF DEMONSTRATION HASHING ---

std::string HashUtils::generateRandomPassword(size_t length) const {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%&";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
    std::string random_string;
    random_string.reserve(length);
    std::generate_n(std::back_inserter(random_string), length,
                    [&]() { return CHARACTERS[distribution(generator)]; });
    return random_string;
}