// include/services/AuthService.hpp
#pragma once

#include <string>
#include <vector>
#include <optional>

#include "models/User.hpp" // Needs full User definition
#include "services/OTPService.hpp" // Needs OTPService definition for member

// Forward declarations for utilities passed as references
class FileHandler;
class HashUtils;

class AuthService {
private:
    std::vector<User>& users;
    FileHandler& fileHandler;
    OTPService& otpService;   // Member variable, needs full type
    HashUtils& hashUtils;     // Member variable, needs full type

public:
    AuthService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref, HashUtils& hu_ref);

    bool registerUser(const std::string& username, const std::string& password,
                      const std::string& fullName, const std::string& email,
                      const std::string& phoneNumber, UserRole role, std::string& outMessage);

    std::optional<User> loginUser(const std::string& username, const std::string& password, std::string& outMessage);

    bool changePassword(const std::string& currentUserId, const std::string& oldPassword,
                        const std::string& newPassword, const std::string& otpCode, std::string& outMessage);

    std::string createAccountWithTemporaryPassword(const std::string& username,
                                                 const std::string& fullName, const std::string& email,
                                                 const std::string& phoneNumber, UserRole role,
                                                 std::string& outMessage);
    
    bool forceTemporaryPasswordChange(User& userToUpdate, const std::string& newPassword, std::string& outMessage);

    std::optional<std::string> setupOtpForUser(const std::string& userId, std::string& outMessage);
    
    bool updateUser(const User& userToUpdate, std::string& outMessage);
    
    bool activateAccount(const std::string& username, std::string& outMessage);

    // Made public for AdminService to use for OTP verification if needed, or AdminService can have its own OTPService ref.
    // Alternatively, OTP verification logic could be more centralized.
    // For now, keeping it here if AdminService needs to call it for a target user.
    // However, it's better if AdminService calls a UserService method which then internally uses OTPService.
    // For direct access if needed by AdminService (e.g. when admin helps user and user provides OTP)
    OTPService& getOtpService() { return otpService; }
    FileHandler& getFileHandler() { return fileHandler; }
};