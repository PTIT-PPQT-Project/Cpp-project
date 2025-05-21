#include "services/UserService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/InputValidator.hpp"
#include <algorithm>
#include <string>

UserService::UserService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref)
    : users(users_ref), fileHandler(fh_ref), otpService(otp_ref) {}

namespace {
    // Helper function to find user by ID
    auto findUserById(std::vector<User>& users, const std::string& userId) {
        return std::find_if(users.begin(), users.end(), 
            [&userId](const User& u) { return u.userId == userId; });
    }

    // Helper function to find user by ID (const version)
    auto findUserByIdConst(const std::vector<User>& users, const std::string& userId) {
        return std::find_if(users.cbegin(), users.cend(), 
            [&userId](const User& u) { return u.userId == userId; });
    }

    // Helper function for sanitizing input strings
    std::string sanitizeInput(const std::string& input) {
        std::string sanitized = input;
        // Remove leading/trailing whitespace
        sanitized.erase(0, sanitized.find_first_not_of(" \t\n\r"));
        sanitized.erase(sanitized.find_last_not_of(" \t\n\r") + 1);
        return sanitized;
    }
}

std::optional<User> UserService::getUserProfile(const std::string& userId) const {
    if (userId.empty()) {
        LOG_WARNING("User profile requested with empty User ID");
        return std::nullopt;
    }

    auto it = findUserByIdConst(users, userId);
    if (it != users.cend()) {
        return *it;
    }
    LOG_WARNING("User profile not found for User ID: " + userId);
    return std::nullopt;
}

std::optional<User> UserService::getUserByUsername(const std::string& username) const {
    if (username.empty()) {
        LOG_WARNING("User profile requested with empty username");
        return std::nullopt;
    }

    auto it = std::find_if(users.cbegin(), users.cend(), 
        [&username](const User& u) { return u.username == username; });
    if (it != users.cend()) {
        return *it;
    }
    LOG_WARNING("User profile not found for username: " + username);
    return std::nullopt;
}

bool UserService::updateUserProfile(const std::string& userId,
                                  const std::string& newFullName,
                                  const std::string& newEmail,
                                  const std::string& newPhoneNumber,
                                  const std::string& otpCode,
                                  std::string& outMessage) {
    if (userId.empty()) {
        outMessage = "Invalid user ID.";
        LOG_WARNING("Profile update failed: empty user ID");
        return false;
    }

    auto it = findUserById(users, userId);
    if (it == users.end()) {
        outMessage = "User account not found.";
        LOG_WARNING("Profile update failed for non-existent user ID: " + userId);
        return false;
    }

    // Store original user data for potential rollback
    User originalUser = *it;

    // OTP Verification
    if (!it->otpSecretKey.empty() && !otpService.verifyOtp(it->otpSecretKey, otpCode)) {
        outMessage = "Invalid OTP code.";
        LOG_WARNING("Profile update failed for user '" + it->username + "': Invalid OTP");
        return false;
    }

    // Validate and update email
    const std::string sanitizedEmail = sanitizeInput(newEmail);
    if (!sanitizedEmail.empty() && it->email != sanitizedEmail) {
        if (!InputValidator::isValidEmail(sanitizedEmail)) {
            outMessage = "Invalid email format.";
            LOG_WARNING("Profile update failed for user '" + it->username + "': " + outMessage);
            return false;
        }
        auto email_exists = std::find_if(users.cbegin(), users.cend(),
            [&sanitizedEmail, &userId](const User& u) {
                return u.email == sanitizedEmail && u.userId != userId;
            });
        if (email_exists != users.cend()) {
            outMessage = "Email address already in use.";
            LOG_WARNING("Profile update failed for user '" + it->username + "': " + outMessage);
            return false;
        }
        it->email = sanitizedEmail;
    }

    // Update full name
    const std::string sanitizedFullName = sanitizeInput(newFullName);
    if (!sanitizedFullName.empty()) {
        it->fullName = sanitizedFullName;
    }

    // Validate and update phone number
    const std::string sanitizedPhoneNumber = sanitizeInput(newPhoneNumber);
    if (!sanitizedPhoneNumber.empty()) {
        if (!InputValidator::isValidPhoneNumber(sanitizedPhoneNumber)) {
            outMessage = "Invalid phone number format.";
            LOG_WARNING("Profile update failed for user '" + it->username + "': " + outMessage);
            return false;
        }
        it->phoneNumber = sanitizedPhoneNumber;
    }

    if (fileHandler.saveUsers(users)) {
        outMessage = "User profile updated successfully.";
        LOG_INFO("Profile updated for user '" + it->username + "'.");
        return true;
    }

    // Rollback changes
    *it = originalUser;
    outMessage = "Failed to save updated user profile.";
    LOG_ERROR(outMessage + " User: " + it->username);
    return false;
}

bool UserService::activateUserAccount(const std::string& userId, std::string& outMessage) {
    if (userId.empty()) {
        outMessage = "Invalid user ID.";
        LOG_WARNING("Account activation failed: empty user ID");
        return false;
    }

    auto it = findUserById(users, userId);
    if (it == users.end()) {
        outMessage = "User not found for activation.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return false;
    }

    if (it->status == AccountStatus::Active) {
        outMessage = "Account is already active.";
        LOG_INFO("Account activation attempted for already active user '" + it->username + "'.");
        return true;
    }

    it->status = AccountStatus::Active;
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account activated successfully.";
        LOG_INFO("Account activated for user '" + it->username + "'.");
        return true;
    }

    it->status = AccountStatus::NotActivated;
    outMessage = "Failed to save account activation status.";
    LOG_ERROR(outMessage + " User: " + it->username);
    return false;
}

bool UserService::deactivateUserAccount(const std::string& userId, std::string& outMessage) {
    if (userId.empty()) {
        outMessage = "Invalid user ID.";
        LOG_WARNING("Account deactivation failed: empty user ID");
        return false;
    }

    auto it = findUserById(users, userId);
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

    auto previousStatus = it->status; // Store previous status for rollback
    it->status = AccountStatus::Inactive;
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account deactivated successfully.";
        LOG_INFO("Account deactivated for user '" + it->username + "'.");
        return true;
    }

    it->status = previousStatus; // Restore previous status
    outMessage = "Failed to save account deactivation status.";
    LOG_ERROR(outMessage + " User: " + it->username);
    return false;
}