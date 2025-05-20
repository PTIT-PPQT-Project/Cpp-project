// src/services/OTPService.cpp
#include "services/OTPService.hpp"
#include "Config.h" // <<< THÊM ĐỂ DÙNG AppConfig::OTP_ISSUER_NAME
#include "utils/Logger.hpp" // For logging
#include <iostream>
#include <random>
#include <algorithm>
#include <vector>


// ... (generateRandomBase32LikeStringPlc không đổi) ...
std::string generateRandomBase32LikeStringPlcInternal(size_t length) { // Đổi tên để tránh trùng
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
    return generateRandomBase32LikeStringPlcInternal(16); // Sử dụng hàm nội bộ đã đổi tên
}

std::string OTPService::generateOtpUri(const std::string& username, const std::string& secretKey) const {
    // Sử dụng AppConfig
    std::string issuer = AppConfig::OTP_ISSUER_NAME;
    return "otpauth://totp/" + issuer + ":" + username + "?secret=" + secretKey + "&issuer=" + issuer;
}

bool OTPService::verifyOtp(const std::string& otpSecretKey, const std::string& userEnteredOtp) const {
    if (otpSecretKey.empty() || userEnteredOtp.empty()) {
        return false;
    }
    LOG_WARNING("OTPService::verifyOtp - Real OTP verification logic is NOT implemented. Using placeholder logic.");
    // Placeholder - KHÔNG AN TOÀN, chỉ cho mục đích demo. Thay thế bằng thư viện OTP thực sự.
    // Ví dụ: return (userEnteredOtp == "123456" && !otpSecretKey.empty());
    return false; // Mặc định là false cho đến khi có logic thực sự
}