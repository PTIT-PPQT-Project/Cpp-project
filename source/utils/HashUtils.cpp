// src/utils/HashUtils.cpp
#include "utils/HashUtils.hpp" // Adjusted include path
#include <random>
#include <sstream>
#include <iomanip> 
#include <algorithm> 
#include <iostream>  // For security warning
#include <chrono>    // For UUID generation part

HashUtils::HashUtils() {
    std::cerr << "**********************************************************************" << std::endl;
    std::cerr << "SECURITY WARNING: HashUtils is using DEMONSTRATION hashing functions." << std::endl;
    std::cerr << "These are NOT cryptographically secure and MUST NOT be used in production." << std::endl;
    std::cerr << "Replace with a proper cryptographic library for password hashing (e.g., bcrypt, Argon2)." << std::endl;
    std::cerr << "**********************************************************************" << std::endl;
}

std::string HashUtils::generateUUID() const {
    std::random_device rd;
    std::mt19937_64 generator(rd() + std::chrono::system_clock::now().time_since_epoch().count()); // Seed with time
    std::uniform_int_distribution<unsigned long long> dist;

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(16) << dist(generator) // 64 bits
       << std::setw(16) << dist(generator); // Another 64 bits for 128-bit representation
    
    std::string R = ss.str();
    // Basic formatting similar to UUID, not RFC compliant
    if (R.length() >= 32) {
        R.insert(8, "-");
        R.insert(13, "-");
        R.insert(18, "-");
        R.insert(23, "-");
        return R.substr(0, 36);
    }
    return R; // Fallback if something unexpected happens
}

std::string HashUtils::generateSalt(size_t length) const {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()[]{}<>~";
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
    // For demo purposes, we'll use a simple concatenation of salt and password
    // In a real application, use a proper cryptographic library like bcrypt or Argon2
    return salt + password;
}

bool HashUtils::verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt) const {
    // For demo purposes, we'll use a simple concatenation comparison
    // In a real application, use a proper cryptographic library like bcrypt or Argon2
    std::string computedHash = salt + password;
    return computedHash == hashedPassword;
}
// --- END OF DEMONSTRATION HASHING ---

std::string HashUtils::generateRandomPassword(size_t length) const {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*";
    if (length == 0) return "";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
    std::string random_string;
    random_string.reserve(length);
    std::generate_n(std::back_inserter(random_string), length,
                    [&]() { return CHARACTERS[distribution(generator)]; });
    return random_string;
}