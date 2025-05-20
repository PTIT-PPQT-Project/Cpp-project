// src/services/OTPService.cpp
#include "services/OTPService.hpp" // Corrected include path
#include "Config.h" // For AppConfig::OTP_ISSUER_NAME
#include <iostream>
#include <random>
#include <algorithm>
#include <vector> // Only if needed for more complex generation

// --- IMPORTANT: REAL OTP LOGIC REQUIRED ---
// The following implementations are placeholders.
// You MUST integrate a proper TOTP library (like cpp-otp or others)
// or implement RFC 6238 and Base32 encoding/decoding correctly and securely.

// Example placeholder for generating a Base32-like random string
std::string generateRandomBase32LikeStringPlc(size_t length) {
    const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
    std::string random_string;
    random_string.reserve(length);
    std::generate_n(std::back_inserter(random_string), length,
                    [&]() { return CHARACTERS[distribution(generator)]; });
    return random_string;
}

OTPService::OTPService() {
    // Constructor
}

std::string OTPService::generateNewOtpSecretKey() const {
    // Placeholder: Generates a 16-character Base32-like string (common length for secrets)
    // Real implementation should use cryptographically secure random byte generation
    // and proper Base32 encoding.
    return generateRandomBase32LikeStringPlc(16);
}

std::string OTPService::generateOtpUri(const std::string& username, const std::string& secretKey) const {
    // otpauth://totp/ISSUER:USERNAME?secret=SECRET_KEY&issuer=ISSUER
    // URL encoding for username and issuer might be needed if they contain special characters.
    std::string issuer = AppConfig::OTP_ISSUER_NAME;
    return "otpauth://totp/" + issuer + ":" + username + "?secret=" + secretKey + "&issuer=" + issuer;
}

bool OTPService::verifyOtp(const std::string& otpSecretKey, const std::string& userEnteredOtp) const {
    if (otpSecretKey.empty() || userEnteredOtp.empty()) {
        return false;
    }
    // --- REAL OTP VERIFICATION LOGIC HERE ---
    // This involves:
    // 1. Base32 decoding the otpSecretKey.
    // 2. Getting the current time window (e.g., 30-second intervals).
    // 3. Calculating the expected OTP using HMAC-SHA1 (or SHA256/512) with the decoded secret and time window.
    // 4. Comparing the calculated OTP with userEnteredOtp.
    // 5. Possibly checking previous/next time windows for clock drift tolerance.

    // Placeholder logic (HIGHLY INSECURE - FOR DEMO ONLY):
    // std::cout << "[OTPService] DEMO: Verifying OTP. Secret (partial): "
    //           << (otpSecretKey.length() > 4 ? otpSecretKey.substr(0, 4) : otpSecretKey)
    //           << "..., User OTP: " << userEnteredOtp << std::endl;
    // if (userEnteredOtp == "123456") return true; // Never do this in production

    LOG_WARNING("OTPService::verifyOtp - Real OTP verification logic is NOT implemented. Using placeholder.");
    // For testing purposes, you might make this return true or false based on some simple rule
    // But ensure it's clear this is not secure.
    // Example: if the last digit of secret key matches last digit of OTP (totally insecure)
    if (!otpSecretKey.empty() && !userEnteredOtp.empty()) {
        if (otpSecretKey.back() == userEnteredOtp.back()){
            // LOG_DEBUG("DEMO OTP verification passed based on last digit match.");
            // return true; // REMOVE THIS INSECURE LOGIC
        }
    }
    // Default to false because no real verification is happening
    return false;
}