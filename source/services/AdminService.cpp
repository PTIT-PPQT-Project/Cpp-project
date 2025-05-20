// src/services/AdminService.cpp
#include "services/AdminService.hpp"
#include "utils/Logger.hpp"
#include "utils/InputValidator.hpp" // For validating email if admin changes it
#include "Config.h"                 // For AppConfig constants if needed (e.g. MASTER_WALLET_ID)
#include <algorithm>               // For std::find_if

AdminService::AdminService(std::vector<User>& u_ref, AuthService& as_ref,
                           UserService& us_ref, WalletService& ws_ref)
    : users(u_ref), authService(as_ref), userService(us_ref), walletService(ws_ref) {}

std::vector<User> AdminService::listAllUsers() const {
    LOG_INFO("Admin listed all users. Total users: " + std::to_string(users.size()));
    return users; // Returns a copy. For large number of users, consider returning const ref
                  // or implementing pagination.
}

bool AdminService::adminCreateUserAccount(const std::string& username, const std::string& fullName,
                                          const std::string& email, const std::string& phoneNumber,
                                          UserRole role, std::string& outCreatedTempPassword,
                                          std::string& outMessage) {
    // Policy: Admin cannot create another Admin user through this general function for safety.
    // This could be configurable or have a separate, more secured function.
    if (role == UserRole::AdminUser) {
        outMessage = "Creating Admin accounts via this interface is restricted for security.";
        LOG_WARNING("Admin attempt to create another Admin account for '" + username + "' was blocked by policy.");
        return false;
    }

    LOG_INFO("Admin attempting to create account. Username: " + username + ", Role: " + User::roleToString(role));
    outCreatedTempPassword = authService.createAccountWithTemporaryPassword(
                                username, fullName, email, phoneNumber, role, outMessage);

    if (!outCreatedTempPassword.empty()) {
        // Successfully created user, now attempt to create a wallet for them.
        std::string newUserId;
        // Find the newly created user in the global g_users vector to get their ID
        // This assumes createAccountWithTemporaryPassword added the user to the 'users' vector AuthService holds.
        auto it_new_user = std::find_if(users.cbegin(), users.cend(), 
                                        [&](const User& u) { return u.username == username; });
        if (it_new_user != users.cend()) {
            newUserId = it_new_user->userId;
            std::string walletMsg;
            if (!walletService.createWalletForUser(newUserId, walletMsg)) {
                LOG_ERROR("Admin created user '" + username + "' successfully, but FAILED to create wallet: " + walletMsg);
                // Append to existing outMessage to inform admin about partial success
                outMessage += " | Wallet creation failed: " + walletMsg;
                // Decide if overall operation is success or failure.
                // For now, user is created, so return true but with a warning in outMessage.
            } else {
                 LOG_INFO("Wallet also created for new user '" + username + "' by admin action: " + walletMsg);
                 outMessage += " | " + walletMsg;
            }
        } else {
            LOG_ERROR("Critical: Could not find newly admin-created user '" + username + "' in user list to create wallet.");
            outMessage += " | Critical error: Could not find user post-creation to initialize wallet.";
        }
        return true; // User account creation part was successful
    }
    // authService.createAccountWithTemporaryPassword failed, outMessage already set.
    return false;
}

bool AdminService::adminUpdateUserProfile(const std::string& adminUserId, // For logging/audit
                                          const std::string& targetUserId,
                                          const std::string& newFullName,
                                          const std::string& newEmail,
                                          const std::string& newPhoneNumber,
                                          AccountStatus newStatus,
                                          const std::string& targetUserOtpCode, // OTP from the target user if they have it
                                          std::string& outMessage) {
    LOG_INFO("Admin (ID: " + adminUserId + ") attempting to update profile for target User ID: " + targetUserId + ".");

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Target user (ID: " + targetUserId + ") not found.";
        LOG_WARNING("Admin update profile failed: " + outMessage);
        return false;
    }
    User currentTargetUserData = targetUserOpt.value(); // Get a copy to check current OTP secret, email etc.

    // If target user has OTP enabled, admin must provide the OTP (given by the target user to admin)
    if (!currentTargetUserData.otpSecretKey.empty()) {
        if (targetUserOtpCode.empty()) {
            outMessage = "Target user (" + currentTargetUserData.username + ") has OTP enabled. Admin must provide user's current OTP code to proceed with changes.";
            LOG_WARNING("Admin update profile for '" + currentTargetUserData.username + "' failed: Missing target user OTP code.");
            return false;
        }
        // Use userService to verify OTP instead of accessing otpService directly
        if (!userService.verifyUserOtp(currentTargetUserData.userId, targetUserOtpCode)) {
            outMessage = "Invalid OTP code provided for target user (" + currentTargetUserData.username + ").";
            LOG_WARNING("Admin update profile for '" + currentTargetUserData.username + "' failed: Invalid target user OTP.");
            return false;
        }
        LOG_INFO("Target user OTP verified for admin update operation on user: " + currentTargetUserData.username);
    }

    // Find the actual user object in the main 'users' vector to modify it
    auto it_target_user_to_modify = std::find_if(users.begin(), users.end(), 
                                     [&](User& u){ return u.userId == targetUserId; });
    
    if (it_target_user_to_modify == users.end()){ // Should not happen if getUserProfile found them
        outMessage = "Internal error: Target user (ID: " + targetUserId + ") found initially but not in updatable master list.";
        LOG_ERROR(outMessage);
        return false;
    }

    bool changesMade = false;

    // Update Full Name
    if (!newFullName.empty() && it_target_user_to_modify->fullName != newFullName) {
        it_target_user_to_modify->fullName = newFullName;
        changesMade = true;
    }

    // Update Email (with validation and uniqueness check)
    if (!newEmail.empty() && it_target_user_to_modify->email != newEmail) {
        if (!InputValidator::isValidEmail(newEmail)) {
            outMessage = "New email format provided is invalid.";
            LOG_WARNING("Admin update profile for '" + it_target_user_to_modify->username + "' failed: " + outMessage);
            return false;
        }
        // Check for email uniqueness against other users
        auto email_exists_elsewhere = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
            return u.email == newEmail && u.userId != targetUserId; // Must not match target user's own ID
        });
        if (email_exists_elsewhere != users.cend()) {
            outMessage = "New email address ('" + newEmail + "') is already in use by another account.";
            LOG_WARNING("Admin update profile for '" + it_target_user_to_modify->username + "' failed: " + outMessage);
            return false;
        }
        it_target_user_to_modify->email = newEmail;
        changesMade = true;
    }

    // Update Phone Number
    if (!newPhoneNumber.empty() && it_target_user_to_modify->phoneNumber != newPhoneNumber) {
        // Add phone number validation if desired using InputValidator
        // if (!InputValidator::isValidPhoneNumber(newPhoneNumber)) { ... }
        it_target_user_to_modify->phoneNumber = newPhoneNumber;
        changesMade = true;
    }

    // Update Account Status (Admin special privilege)
    if (it_target_user_to_modify->status != newStatus) {
        it_target_user_to_modify->status = newStatus;
        changesMade = true;
    }
    
    if (!changesMade) {
        outMessage = "No changes were specified for user '" + it_target_user_to_modify->username + "'.";
        LOG_INFO(outMessage);
        return true; // Operation considered successful as no changes were requested/needed.
    }

    // Persist changes using userService instead of accessing fileHandler directly
    if (userService.saveUserChanges()) {
        outMessage = "Admin successfully updated user profile for '" + it_target_user_to_modify->username + "'.";
        LOG_INFO(outMessage + " (Admin: " + adminUserId + ")");
        return true;
    } else {
        outMessage = "Error saving updated user profile by admin for '" + it_target_user_to_modify->username + "'.";
        LOG_ERROR(outMessage + " Changes might be in memory but not persisted.");
        return false;
    }
}

bool AdminService::adminActivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin attempting to activate user ID '" + targetUserId + "'.");
    return userService.activateUserAccount(targetUserId, outMessage); // Delegate to UserService
}

bool AdminService::adminDeactivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin attempting to deactivate user ID '" + targetUserId + "'.");
    return userService.deactivateUserAccount(targetUserId, outMessage); // Delegate to UserService
}

bool AdminService::adminDepositToUserWallet(const std::string& adminUserId, const std::string& targetUserId,
                                            double amount, const std::string& reason, std::string& outMessage) {
    LOG_INFO("Admin (ID: " + adminUserId + ") attempting to deposit " + std::to_string(amount) +
             " to target User ID '" + targetUserId + "' for reason: '" + reason + "'.");

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Target user (ID: " + targetUserId + ") for deposit not found.";
        LOG_WARNING("Admin deposit failed: " + outMessage);
        return false;
    }
    User targetUser = targetUserOpt.value();

    auto targetWalletOpt = walletService.getWalletByUserId(targetUserId);
    if (!targetWalletOpt) {
        outMessage = "Wallet for target user '" + targetUser.username + "' not found. Cannot deposit points.";
        LOG_WARNING("Admin deposit failed: " + outMessage);
        // Admin could be given an option to create a wallet here if that's a desired feature.
        return false;
    }

    std::string depositDescription = "Admin deposit (by " + adminUserId + "): " + reason;
    // Use a configured Master Wallet ID or a generic system source ID for admin deposits
    return walletService.depositPoints(targetWalletOpt.value().walletId, amount, depositDescription, 
                                       adminUserId, // Record who initiated
                                       outMessage, AppConfig::MASTER_WALLET_ID);
}