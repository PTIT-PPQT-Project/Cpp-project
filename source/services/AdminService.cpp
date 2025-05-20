// src/services/AdminService.cpp
#include "services/AdminService.hpp"
#include "utils/Logger.hpp"
#include "Config.h" // For AppConfig::MASTER_WALLET_ID if used

AdminService::AdminService(std::vector<User>& u_ref, AuthService& as_ref, 
                           UserService& us_ref, WalletService& ws_ref)
    : users(u_ref), authService(as_ref), userService(us_ref), walletService(ws_ref) {}

std::vector<User> AdminService::listAllUsers() const {
    LOG_INFO("Admin listed all users.");
    return users; // Returns a copy. For performance on large sets, consider const ref or specific DTOs.
}

bool AdminService::adminCreateUserAccount(const std::string& username, const std::string& fullName,
                                          const std::string& email, const std::string& phoneNumber,
                                          UserRole role, std::string& outCreatedTempPassword, 
                                          std::string& outMessage) {
    // Policy: Admin cannot create another Admin user through this simple function for safety.
    if (role == UserRole::AdminUser) {
        outMessage = "Cannot create Admin accounts using this function for security reasons.";
        LOG_WARNING("Admin attempt to create another Admin account for '" + username + "' was blocked.");
        return false;
    }
    outCreatedTempPassword = authService.createAccountWithTemporaryPassword(
                                username, fullName, email, phoneNumber, role, outMessage);
    
    if (!outCreatedTempPassword.empty()) {
        // Auto-create wallet for the new user
        std::string newUserId;
        for(const auto& u : users) { // Find the newly created user to get ID
            if(u.username == username) {
                newUserId = u.userId;
                break;
            }
        }
        if (!newUserId.empty()) {
            std::string walletMsg;
            if (!walletService.createWalletForUser(newUserId, walletMsg)) {
                LOG_ERROR("Admin created user '" + username + "' but failed to create wallet: " + walletMsg);
                // User created, but wallet failed. This is a partial success/failure state.
                // outMessage might need to reflect this.
                outMessage += " User created, but wallet creation failed: " + walletMsg;
            } else {
                 LOG_INFO("Wallet created for new user '" + username + "' by admin action.");
            }
        } else {
            LOG_ERROR("Could not find newly created user '" + username + "' by admin to create wallet.");
            outMessage += " User created, but could not find user to create wallet.";
        }
        return true;
    }
    return false;
}

bool AdminService::adminUpdateUserProfile(const std::string& adminUserId, const std::string& targetUserId,
                                          const std::string& newFullName, const std::string& newEmail,
                                          const std::string& newPhoneNumber, AccountStatus newStatus,
                                          const std::string& targetUserOtpCode,
                                          std::string& outMessage) {
    LOG_INFO("Admin '" + adminUserId + "' attempting to update profile for user ID '" + targetUserId + "'.");

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Target user not found.";
        LOG_WARNING("Admin update profile failed: " + outMessage + " Target ID: " + targetUserId);
        return false;
    }
    User targetUser = targetUserOpt.value(); // Get a copy for checking OTP secret

    // If target user has OTP enabled, admin needs to provide the OTP obtained from the user.
    if (!targetUser.otpSecretKey.empty()) {
        if (targetUserOtpCode.empty()) {
            outMessage = "Target user has OTP enabled. Admin must provide user's OTP code.";
            LOG_WARNING("Admin update profile for '" + targetUser.username + "' failed: Missing target user OTP.");
            return false;
        }
        // Use the OTPService instance from AuthService (or pass one to AdminService)
        if (!authService.otpService.verifyOtp(targetUser.otpSecretKey, targetUserOtpCode)) {
            outMessage = "Invalid OTP code for target user.";
            LOG_WARNING("Admin update profile for '" + targetUser.username + "' failed: Invalid target user OTP.");
            return false;
        }
    }

    // Now, actually find the user in the main vector to update
    auto it_target = std::find_if(users.begin(), users.end(), [&](User& u){ return u.userId == targetUserId; });
    if (it_target == users.end()){ 
        outMessage = "Internal error: Target user found initially but not in updatable list.";
        LOG_ERROR(outMessage + " Target ID: " + targetUserId);
        return false;
    }

    // Update fields
    bool changed = false;
    if (!newFullName.empty() && it_target->fullName != newFullName) {
        it_target->fullName = newFullName;
        changed = true;
    }
    if (!newPhoneNumber.empty() && it_target->phoneNumber != newPhoneNumber) {
        it_target->phoneNumber = newPhoneNumber;
        changed = true;
    }
    if (it_target->status != newStatus) {
        it_target->status = newStatus;
        changed = true;
    }

    if (!newEmail.empty() && it_target->email != newEmail) {
        if (!InputValidator::isValidEmail(newEmail)) {
            outMessage = "New email format is invalid.";
             LOG_WARNING("Admin update profile for '" + it_target->username + "' failed: " + outMessage);
            return false;
        }
        auto email_exists = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
            return u.email == newEmail && u.userId != targetUserId;
        });
        if (email_exists != users.cend()) {
            outMessage = "New email address is already in use by another account.";
            LOG_WARNING("Admin update profile for '" + it_target->username + "' failed: " + outMessage);
            return false;
        }
        it_target->email = newEmail;
        changed = true;
    }
    
    if (!changed) {
        outMessage = "No changes provided for user profile.";
        LOG_INFO("Admin update profile for '" + it_target->username + "': No actual changes made.");
        return true; // Or false if "no change" is an issue
    }

    if (fileHandler.saveUsers(users)) { // Use fileHandler from one of the services, e.g. authService
        outMessage = "Admin successfully updated user profile for " + it_target->username + ".";
        LOG_INFO(outMessage);
        return true;
    } else {
        outMessage = "Error saving updated user profile by admin.";
        LOG_ERROR(outMessage + " Target User: " + it_target->username);
        // Consider rollback for in-memory changes.
        return false;
    }
}
    
bool AdminService::adminActivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin attempting to activate user ID '" + targetUserId + "'.");
    return userService.activateUserAccount(targetUserId, outMessage);
}

bool AdminService::adminDeactivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin attempting to deactivate user ID '" + targetUserId + "'.");
    return userService.deactivateUserAccount(targetUserId, outMessage);
}

bool AdminService::adminDepositToUserWallet(const std::string& adminUserId, const std::string& targetUserId, 
                                            double amount, const std::string& reason, std::string& outMessage) {
    LOG_INFO("Admin '" + adminUserId + "' attempting to deposit " + std::to_string(amount) + 
             " to user ID '" + targetUserId + "' for reason: " + reason);

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Target user for deposit not found.";
        LOG_WARNING("Admin deposit failed: " + outMessage + " Target User ID: " + targetUserId);
        return false;
    }
    auto targetWalletOpt = walletService.getWalletByUserId(targetUserId);
    if (!targetWalletOpt) {
        outMessage = "Wallet for target user not found. Please ensure user has a wallet.";
        LOG_WARNING("Admin deposit failed: " + outMessage + " Target User: " + targetUserOpt.value().username);
        // Option: Admin could trigger wallet creation here if desired.
        // std::string walletCreationMsg;
        // if (!walletService.createWalletForUser(targetUserId, walletCreationMsg)) {
        //    outMessage = "Target user has no wallet, and attempt to create one failed: " + walletCreationMsg;
        //    return false;
        // }
        // targetWalletOpt = walletService.getWalletByUserId(targetUserId);
        // if (!targetWalletOpt) { /* should not happen if creation succeeded */ return false; }
        return false;
    }

    std::string description = "Admin deposit (" + adminUserId + "): " + reason;
    // Using MASTER_WALLET_ID or a generic system source ID for deposits from admin
    return walletService.depositPoints(targetWalletOpt.value().walletId, amount, description, adminUserId, 
                                       outMessage, AppConfig::MASTER_WALLET_ID); 
}