// src/services/UserService.cpp
#include "services/UserService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/InputValidator.hpp" // For validating new email if changed
#include <algorithm>

UserService::UserService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref)
    : users(users_ref), fileHandler(fh_ref), otpService(otp_ref) {}

std::optional<User> UserService::getUserProfile(const std::string& userId) const {
    auto it = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
        return u.userId == userId;
    });
    if (it != users.cend()) {
        return *it;
    }
    LOG_WARNING("User profile requested for non-existent User ID: " + userId);
    return std::nullopt;
}

std::optional<User> UserService::getUserByUsername(const std::string& username) const {
    auto it = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
        return u.username == username;
    });
    if (it != users.cend()) {
        return *it;
    }
    LOG_WARNING("User profile requested for non-existent username: " + username);
    return std::nullopt;
}

bool UserService::updateUserProfile(const std::string& userId,
                                    const std::string& newFullName,
                                    const std::string& newEmail,
                                    const std::string& newPhoneNumber,
                                    const std::string& otpCode,
                                    std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == userId;
    });

    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        return false;
    }

    // OTP Verification if user has OTP enabled
    if (!it->otpSecretKey.empty()) {
        if (otpCode.empty()) {
            outMessage = "Ban can nhap ma OTP de xac nhan thay doi.";
            return false;
        }
        if (!otpService.verifyOtp(it->otpSecretKey, otpCode)) {
            outMessage = "Ma OTP khong hop le.";
            return false;
        }
    }

    // Validate and update email if it's being changed
    if (!newEmail.empty() && it->email != newEmail) {
        if (!InputValidator::isValidEmail(newEmail)) {
            outMessage = "New email format is invalid.";
            LOG_WARNING("Profile update failed for user '" + it->username + "': " + outMessage);
            return false;
        }
        // Check for email uniqueness
        auto email_exists = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
            return u.email == newEmail && u.userId != userId;
        });
        if (email_exists != users.cend()) {
            outMessage = "New email address is already in use by another account.";
            LOG_WARNING("Profile update failed for user '" + it->username + "': " + outMessage);
            return false;
        }
        it->email = newEmail;
    }

    // Update other fields
    if (!newFullName.empty()) {
        it->fullName = newFullName;
    }
    if (!newPhoneNumber.empty()) {
        // Add phone number validation if desired
        // if (!InputValidator::isValidPhoneNumber(newPhoneNumber)) { ... }
        it->phoneNumber = newPhoneNumber;
    }

    if (fileHandler.saveUsers(users)) {
        outMessage = "User profile updated successfully.";
        LOG_INFO("Profile updated for user '" + it->username + "'.");
        return true;
    } else {
        outMessage = "Error saving updated user profile.";
        LOG_ERROR(outMessage + " User: " + it->username);
        // Consider rollback for in-memory changes if save fails.
        return false;
    }
}

bool UserService::activateUserAccount(const std::string& userId, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == userId;
    });
    if (it == users.end()) {
        outMessage = "User not found for activation.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return false;
    }
    if (it->status == AccountStatus::Active) {
        outMessage = "Account is already active.";
        LOG_INFO("Account activation attempted for already active user '" + it->username + "'.");
        return true; // No change needed, considered success
    }
    it->status = AccountStatus::Active;
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account activated successfully.";
        LOG_INFO("Account activated for user '" + it->username + "'.");
        return true;
    }
    outMessage = "Error saving account activation status.";
    LOG_ERROR(outMessage + " User: " + it->username);
    it->status = AccountStatus::NotActivated; // Rollback in-memory
    return false;
}

bool UserService::deactivateUserAccount(const std::string& userId, std::string& outMessage) {
     auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == userId;
    });
    if (it == users.end()) {
        outMessage = "User not found for deactivation.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return false;
    }
    if (it->status == AccountStatus::Inactive) {
        outMessage = "Account is already inactive.";
        LOG_INFO("Account deactivation attempted for already inactive user '" + it->username + "'.");
        return true;
    }
    it->status = AccountStatus::Inactive;
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account deactivated successfully.";
        LOG_INFO("Account deactivated for user '" + it->username + "'.");
        return true;
    }
    outMessage = "Error saving account deactivation status.";
    LOG_ERROR(outMessage + " User: " + it->username);
    it->status = AccountStatus::Active; // Rollback in-memory (assuming previous state was Active)
    return false;
}