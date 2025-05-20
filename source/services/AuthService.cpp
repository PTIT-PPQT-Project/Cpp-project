// src/services/AuthService.cpp
#include "services/AuthService.hpp" // Assumes include path is set for "services/" directory
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/Logger.hpp"
#include "Config.h" // For AppConfig constants like MIN_PASSWORD_LENGTH
#include "utils/TimeUtils.hpp"  // Add missing include for TimeUtils
#include <algorithm>
#include <ctime>     // For std::time
#include <vector>    // For std::vector

AuthService::AuthService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref, HashUtils& hu_ref)
    : users(users_ref), fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

bool AuthService::registerUser(const std::string& username, const std::string& password,
                               const std::string& fullName, const std::string& email,
                               const std::string& phoneNumber, UserRole role, std::string& outMessage) {
    // Validate username and email uniqueness
    auto it_user = std::find_if(users.begin(), users.end(), [&](const User& u) {
        return u.username == username || u.email == email;
    });

    if (it_user != users.end()) {
        outMessage = (it_user->username == username) ? "Username already exists." : "Email already exists.";
        LOG_WARNING("Registration failed for '" + username + "': " + outMessage);
        return false;
    }

    // Validate password length (using AppConfig)
    if (password.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        outMessage = "Password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters long.";
        LOG_WARNING("Registration failed for '" + username + "': " + outMessage);
        return false;
    }
    // More comprehensive password validation should be done here or via InputValidator

    User newUser;
    newUser.userId = hashUtils.generateUUID(); // Generate a unique ID
    newUser.username = username;
    newUser.passwordSalt = hashUtils.generateSalt();
    newUser.hashedPassword = hashUtils.hashPassword(password, newUser.passwordSalt);
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::NotActivated; // Default status, admin might activate or email verification
    newUser.isTemporaryPassword = false;
    newUser.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newUser.lastLoginTimestamp = 0; // Not logged in yet

    users.push_back(newUser);
    if (fileHandler.saveUsers(users)) {
        outMessage = "Registration successful! Account created for " + username + ". Please wait for activation.";
        LOG_INFO(outMessage);
        return true;
    } else {
        outMessage = "Error saving user data during registration.";
        LOG_ERROR(outMessage + " Rolling back user addition for " + username);
        users.pop_back(); // Rollback the addition if save fails
        return false;
    }
}

std::optional<User> AuthService::loginUser(const std::string& username, const std::string& password, std::string& outMessage) {
    auto it_user = std::find_if(users.begin(), users.end(), [&](const User& u) {
        return u.username == username;
    });

    if (it_user == users.end()) {
        outMessage = "Username not found.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }

    if (it_user->status == AccountStatus::Inactive) {
        outMessage = "Account is inactive. Please contact support.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }

    if (it_user->status == AccountStatus::NotActivated) {
        outMessage = "Account is not activated yet. Please check your email or contact support.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }

    bool passwordValid = false;
    if (it_user->isTemporaryPassword) {
        // For temporary passwords, compare directly
        passwordValid = (password == it_user->hashedPassword);
        if (!passwordValid) {
            outMessage = "Incorrect temporary password. Please use the temporary password provided by your administrator.";
        }
    } else {
        // For regular passwords, use hash verification
        passwordValid = hashUtils.verifyPassword(password, it_user->hashedPassword, it_user->passwordSalt);
        if (!passwordValid) {
            outMessage = "Incorrect password.";
        }
    }

    if (!passwordValid) {
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }

    // Update last login timestamp
    it_user->lastLoginTimestamp = TimeUtils::getCurrentTimestamp();
    if (!fileHandler.saveUsers(users)) {
        LOG_ERROR("Failed to update last login timestamp for user '" + username + "'");
    }

    LOG_INFO("User '" + username + "' logged in successfully.");
    LOG_INFO("User '" + username + "' logged in.");
    return *it_user; // Return the User object
}

bool AuthService::changePassword(const std::string& currentUserId, const std::string& oldPassword,
                                   const std::string& newPassword, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) { // Note: User& for modification
        return u.userId == currentUserId;
    });

    if (it == users.end()) {
        outMessage = "User not found for password change.";
        LOG_WARNING("Change password attempt failed: " + outMessage + " (User ID: " + currentUserId + ")");
        return false;
    }

    if (!hashUtils.verifyPassword(oldPassword, it->hashedPassword, it->passwordSalt)) {
        outMessage = "Old password incorrect.";
        LOG_WARNING("Change password attempt failed for user '" + it->username + "': " + outMessage);
        return false;
    }

    // Check if new password is the same as old password
    if (oldPassword == newPassword) {
        outMessage = "New password must be different from the old password.";
        LOG_WARNING("Change password attempt failed for user '" + it->username + "': " + outMessage);
        return false;
    }

    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) { // Use AppConfig
        outMessage = "New password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters.";
        LOG_WARNING("Change password attempt failed for user '" + it->username + "': " + outMessage);
        return false;
    }
    // Add more comprehensive password validation here if needed (e.g., from InputValidator)

    // Re-use the existing salt
    it->hashedPassword = hashUtils.hashPassword(newPassword, it->passwordSalt);
    it->isTemporaryPassword = false; // Any password change makes it non-temporary

    if (fileHandler.saveUsers(users)) {
        outMessage = "Password changed successfully.";
        LOG_INFO("Password changed for user '" + it->username + "'.");
        return true;
    } else {
        outMessage = "Error saving password changes.";
        LOG_ERROR(outMessage + " User: " + it->username);
        // Consider rollback if save fails - complex for file-based.
        // For now, data in memory is changed but not persisted.
        return false;
    }
}

std::string AuthService::createAccountWithTemporaryPassword(const std::string& username,
                                                            const std::string& fullName, const std::string& email,
                                                            const std::string& phoneNumber, UserRole role,
                                                            std::string& outMessage) {
    auto it_user = std::find_if(users.begin(), users.end(), [&](const User& u) {
        return u.username == username || u.email == email;
    });
    if (it_user != users.end()) {
        outMessage = (it_user->username == username) ? "Username already exists." : "Email already exists.";
        LOG_WARNING("Admin create account with temp pass failed for '" + username + "': " + outMessage);
        return ""; // Return empty string on failure
    }

    // Generate a random temporary password
    std::string tempPassword = hashUtils.generateRandomPassword(AppConfig::MIN_PASSWORD_LENGTH);

    User newUser;
    newUser.userId = hashUtils.generateUUID();
    newUser.username = username;
    newUser.passwordSalt = hashUtils.generateSalt();
    // Store the temporary password directly in hashedPassword
    newUser.hashedPassword = tempPassword; // Store the actual temporary password, not hashed
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::Active;
    newUser.isTemporaryPassword = true;
    newUser.creationTimestamp = TimeUtils::getCurrentTimestamp();

    users.push_back(newUser);
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account created successfully with a temporary password.";
        LOG_INFO("Admin created account for '" + username + "' with temporary password.");
        return tempPassword; // Return the temporary password
    } else {
        outMessage = "Error saving new user data (with temp pass).";
        LOG_ERROR(outMessage + " Rolling back admin user creation for " + username);
        users.pop_back();
        return "";
    }
}

bool AuthService::forceTemporaryPasswordChange(User& userToUpdate, const std::string& newPassword, std::string& outMessage) {
    // Find the user in the main vector 'users' to ensure we update the persisted version
    auto it = std::find_if(users.begin(), users.end(), [&](const User& u_db) {
        return u_db.userId == userToUpdate.userId;
    });

    if (it == users.end()){
        outMessage = "User not found in database for temporary password change.";
        LOG_ERROR(outMessage + " User ID: " + userToUpdate.userId);
        return false;
    }

    if (!it->isTemporaryPassword) {
        outMessage = "Account is not using a temporary password or it has already been changed.";
        LOG_INFO(outMessage + " User: " + it->username);
        return true; // Not an error, just no action needed. Or false if strict.
    }
    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) { // Use AppConfig
        outMessage = "New password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters.";
        LOG_WARNING("Temp password change failed for user '" + it->username + "': " + outMessage);
        return false;
    }
    // Add more comprehensive password validation here if needed

    it->hashedPassword = hashUtils.hashPassword(newPassword, it->passwordSalt);
    it->isTemporaryPassword = false;

    // Update the passed-in reference 'userToUpdate' as well, if it's used (e.g., g_currentUser)
    userToUpdate.isTemporaryPassword = false;
    userToUpdate.hashedPassword = it->hashedPassword;

    if (fileHandler.saveUsers(users)) {
        outMessage = "Temporary password changed successfully.";
        LOG_INFO("Temporary password changed for user '" + it->username + "'.");
        return true;
    } else {
        outMessage = "Error saving new password after temporary change.";
        LOG_ERROR(outMessage + " User: " + it->username + ". Password changed in memory but not saved.");
        // Attempt to rollback in-memory change (this is tricky without original hash)
        it->isTemporaryPassword = true; // Revert flag
        // Cannot easily revert hashedPassword without storing it temporarily.
        // This highlights the need for transactional saves or better error handling.
        return false;
    }
}

std::optional<std::string> AuthService::setupOtpForUser(const std::string& userId, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) { // User& for modification
        return u.userId == userId;
    });

    if (it == users.end()) {
        outMessage = "User not found for OTP setup.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return std::nullopt;
    }

    if (!it->otpSecretKey.empty()) {
        outMessage = "OTP is already set up for this user. To change, please disable OTP first (feature not implemented).";
        LOG_INFO("OTP setup attempt for user '" + it->username + "' but OTP already exists. Secret not returned.");
        return std::nullopt; // Do not return existing secret here for security, user should re-verify if they lost it
    }

    std::string newSecret = otpService.generateNewOtpSecretKey();
    it->otpSecretKey = newSecret;

    if (fileHandler.saveUsers(users)) {
        outMessage = "OTP setup successful. Please save this secret key securely. It will not be shown again.";
        LOG_INFO("OTP setup successful for user '" + it->username + "'.");
        return newSecret; // Return the new secret for the user to scan/input
    } else {
        outMessage = "Error saving OTP secret key.";
        LOG_ERROR(outMessage + " User: " + it->username);
        it->otpSecretKey = ""; // Rollback in-memory change
        return std::nullopt;
    }
}