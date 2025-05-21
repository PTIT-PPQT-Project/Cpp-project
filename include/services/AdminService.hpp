// include/services/AdminService.hpp
#pragma once

#include <string>
#include <vector>
#include <optional>
#include "models/User.hpp"
#include "services/AuthService.hpp"    // Needs full definitions for members
#include "services/UserService.hpp"
#include "services/WalletService.hpp"

class AdminService {
private:
    std::vector<User>& users; // For direct listing if needed, though services should provide views
    AuthService& authService;
    UserService& userService;
    WalletService& walletService;

public:
    AdminService(std::vector<User>& u_ref, AuthService& as_ref, 
                 UserService& us_ref, WalletService& ws_ref);

    std::vector<User> listAllUsers() const;

    bool adminCreateUserAccount(const std::string& username, const std::string& fullName,
                                const std::string& email, const std::string& phoneNumber,
                                UserRole role, std::string& outCreatedTempPassword, 
                                std::string& outMessage);

    bool adminUpdateUserProfile(const std::string& adminUserId, // For logging/audit
                                const std::string& targetUserId,
                                const std::string& newFullName,
                                const std::string& newEmail,
                                const std::string& newPhoneNumber,
                                AccountStatus newStatus, // Admin can change status
                                const std::string& targetUserOtpCode, // OTP of the target user, if they have it enabled
                                std::string& outMessage);
    
    bool adminActivateUser(const std::string& targetUserId, std::string& outMessage);
    bool adminDeactivateUser(const std::string& targetUserId, std::string& outMessage);

    bool adminDepositToUserWallet(const std::string& adminUserId, // For logging/audit
                                  const std::string& targetUserId, double amount, 
                                  const std::string& reason, std::string& outMessage);
};