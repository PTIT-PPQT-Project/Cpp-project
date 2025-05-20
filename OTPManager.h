#ifndef OTP_MANAGER_H
#define OTP_MANAGER_H

#include <string>
#include <chrono>
#include <map>

class OTPManager {
private:
    static OTPManager* instance;
    std::map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> otpStore;
    
    OTPManager() = default;
    std::string generateOTP();
    
public:
    static OTPManager* getInstance();
    
    // Generate and store OTP for a user
    std::string generateAndStoreOTP(const std::string& username);
    
    // Verify OTP for a user
    bool verifyOTP(const std::string& username, const std::string& otp);
    
    // Remove expired OTPs
    void cleanupExpiredOTPs();
};

#endif 