#include "OTPManager.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

OTPManager* OTPManager::instance = nullptr;

OTPManager* OTPManager::getInstance() {
    if (instance == nullptr) {
        instance = new OTPManager();
    }
    return instance;
}

std::string OTPManager::generateOTP() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 999999);
    
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << dis(gen);
    return ss.str();
}

std::string OTPManager::generateAndStoreOTP(const std::string& username) {
    std::string otp = generateOTP();
    auto now = std::chrono::system_clock::now();
    otpStore[username] = std::make_pair(otp, now);
    return otp;
}

bool OTPManager::verifyOTP(const std::string& username, const std::string& otp) {
    auto it = otpStore.find(username);
    if (it == otpStore.end()) {
        return false;
    }
    
    auto now = std::chrono::system_clock::now();
    auto otpTime = it->second.second;
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - otpTime);
    
    // OTP expires after 5 minutes
    if (duration.count() > 5) {
        otpStore.erase(it);
        return false;
    }
    
    bool isValid = (it->second.first == otp);
    if (isValid) {
        otpStore.erase(it);
    }
    return isValid;
}

void OTPManager::cleanupExpiredOTPs() {
    auto now = std::chrono::system_clock::now();
    for (auto it = otpStore.begin(); it != otpStore.end();) {
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(
            now - it->second.second);
        if (duration.count() > 5) {
            it = otpStore.erase(it);
        } else {
            ++it;
        }
    }
} 