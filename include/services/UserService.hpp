// include/services/UserService.hpp
#pragma once

#include <string>
#include <vector>
#include <optional>
#include "models/User.hpp"
#include "services/OTPService.hpp" // For OTP verification during updates

// Forward declaration for FileHandler
class FileHandler;

class UserService {
private:
    std::vector<User>& users;
    FileHandler& fileHandler;
    OTPService& otpService;

public:
    UserService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref);

    std::optional<User> getUserProfile(const std::string& userId) const;

    bool updateUserProfile(const std::string& userId,
                           const std::string& newFullName,
                           const std::string& newEmail, // Needs uniqueness check if changed
                           const std::string& newPhoneNumber,
                           const std::string& otpCode, // Empty if OTP not required or not set up
                           std::string& outMessage);
    
    bool activateUserAccount(const std::string& userId, std::string& outMessage);
    bool deactivateUserAccount(const std::string& userId, std::string& outMessage);

    // New methods
    bool verifyUserOtp(const std::string& userId, const std::string& otpCode) const;
    bool saveUserChanges();
};