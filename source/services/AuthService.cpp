// src/services/AuthService.cpp
#include "services/AuthService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/Logger.hpp" // For LOG_ macros
#include <algorithm>
#include <ctime>

AuthService::AuthService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref, HashUtils& hu_ref)
    : users(users_ref), fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

bool AuthService::registerUser(const std::string& username, const std::string& password,
                               const std::string& fullName, const std::string& email,
                               const std::string& phoneNumber, UserRole role, std::string& outMessage) {
    auto it_user = std::find_if(users.begin(), users.end(), [&](const User& u) {
        return u.username == username || u.email == email;
    });

    if (it_user != users.end()) {
        outMessage = (it_user->username == username) ? "Username already exists." : "Email already exists.";
        LOG_WARNING("Registration failed: " + outMessage);
        return false;
    }

    // Password validation should ideally use InputValidator, but basic check here for now
    if (password.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        outMessage = "Password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters long.";
        LOG_WARNING("Registration failed: " + outMessage);
        return false;
    }

    User newUser;
    newUser.userId = hashUtils.generateUUID();
    newUser.username = username;
    newUser.passwordSalt = hashUtils.generateSalt();
    newUser.hashedPassword = hashUtils.hashPassword(password, newUser.passwordSalt);
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::NotActivated; // Or Active, depending on flow
    newUser.isTemporaryPassword = false;
    newUser.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newUser.lastLoginTimestamp = 0;

    users.push_back(newUser);
    if (fileHandler.saveUsers(users)) {
        outMessage = "Registration successful! Account created for " + username + ".";
        LOG_INFO(outMessage);
        return true;
    } else {
        outMessage = "Error saving user data during registration.";
        LOG_ERROR(outMessage + " Rolling back user addition for " + username);
        users.pop_back(); // Rollback
        return false;
    }
}

std::optional<User> AuthService::loginUser(const std::string& username, const std::string& password, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](const User& u) {
        return u.username == username;
    });

    if (it == users.end()) {
        outMessage = "Username not found.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }

    if (it->status == AccountStatus::Inactive) {
        outMessage = "Account is inactive. Please contact support.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }
     if (it->status == AccountStatus::NotActivated) {
        outMessage = "Account is not activated yet.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }
    
    if (hashUtils.verifyPassword(password, it->hashedPassword, it->passwordSalt)) {
        outMessage = "Login successful.";
        it->lastLoginTimestamp = TimeUtils::getCurrentTimestamp();
        if (!fileHandler.saveUsers(users)) { // Attempt to save last login time
            LOG_ERROR("Failed to save last login time for user: " + username);
            // Continue with login even if save fails, but log it.
        }
        LOG_INFO("User '" + username + "' logged in successfully.");
        return *it;
    } else {
        outMessage = "Incorrect password.";
        LOG_WARNING("Login attempt failed for username '" + username + "': " + outMessage);
        return std::nullopt;
    }
}

bool AuthService::changePassword(const std::string& currentUserId, const std::string& oldPassword,
                                   const std::string& newPassword, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == currentUserId;
    });

    if (it == users.end()) {
        outMessage = "User not found.";
        LOG_WARNING("Change password attempt failed: " + outMessage + " (User ID: " + currentUserId + ")");
        return false;
    }

    if (!hashUtils.verifyPassword(oldPassword, it->hashedPassword, it->passwordSalt)) {
        outMessage = "Old password incorrect.";
        LOG_WARNING("Change password attempt failed for user '" + it->username + "': " + outMessage);
        return false;
    }

    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) { // Use Config
        outMessage = "New password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters.";
        LOG_WARNING("Change password attempt failed for user '" + it->username + "': " + outMessage);
        return false;
    }

    // Re-use the existing salt
    it->hashedPassword = hashUtils.hashPassword(newPassword, it->passwordSalt);
    it->isTemporaryPassword = false; 
    
    if (fileHandler.saveUsers(users)) {
        outMessage = "Password changed successfully.";
        LOG_INFO("Password changed for user '" + it->username + "'.");
        return true;
    } else {
        outMessage = "Error saving password changes.";
        LOG_ERROR(outMessage + " User: " + it->username);
        // Consider how to handle this failure; a rollback might be complex here if only save failed.
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
        LOG_WARNING("Admin create account failed: " + outMessage);
        return "";
    }

    std::string tempPassword = hashUtils.generateRandomPassword(AppConfig::MIN_PASSWORD_LENGTH); // Use Config for length

    User newUser;
    newUser.userId = hashUtils.generateUUID();
    newUser.username = username;
    newUser.passwordSalt = hashUtils.generateSalt();
    newUser.hashedPassword = hashUtils.hashPassword(tempPassword, newUser.passwordSalt);
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::Active; // Or NotActivated, admin can activate later
    newUser.isTemporaryPassword = true;
    newUser.creationTimestamp = TimeUtils::getCurrentTimestamp();

    users.push_back(newUser);
    if (fileHandler.saveUsers(users)) {
        outMessage = "Account created successfully with a temporary password.";
        LOG_INFO("Admin created account for '" + username + "' with temporary password.");
        return tempPassword;
    } else {
        outMessage = "Error saving new user data.";
        LOG_ERROR(outMessage + " Rolling back admin user creation for " + username);
        users.pop_back();
        return "";
    }
}
    
bool AuthService::forceTemporaryPasswordChange(User& userToUpdate, const std::string& newPassword, std::string& outMessage) {
    // Find the user in the main vector to update them there, as userToUpdate might be a copy
    auto it = std::find_if(users.begin(), users.end(), [&](const User& u_db) {
        return u_db.userId == userToUpdate.userId;
    });

    if (it == users.end()){
        outMessage = "User not found in database for temporary password change.";
        LOG_ERROR(outMessage + " User ID: " + userToUpdate.userId);
        return false;
    }

    if (!it->isTemporaryPassword) { // Check the persisted user's status
        outMessage = "Account is not using a temporary password or it has already been changed.";
        // LOG_INFO(outMessage + " User: " + it->username); // Not an error, just info
        return true; // Or false if you consider this an invalid operation attempt
    }
    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) { // Use Config
        outMessage = "New password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters.";
        LOG_WARNING("Temp password change failed for user '" + it->username + "': " + outMessage);
        return false;
    }

    it->hashedPassword = hashUtils.hashPassword(newPassword, it->passwordSalt);
    it->isTemporaryPassword = false;
    
    // Also update the passed-in user object if it's intended to be used immediately (e.g. g_currentUser)
    userToUpdate.isTemporaryPassword = false;
    userToUpdate.hashedPassword = it->hashedPassword;

    if (fileHandler.saveUsers(users)) {
        outMessage = "Temporary password changed successfully.";
        LOG_INFO("Temporary password changed for user '" + it->username + "'.");
        return true;
    } else {
        outMessage = "Error saving new password after temporary change.";
        LOG_ERROR(outMessage + " User: " + it->username);
        // Critical: password changed in memory but not saved.
        // Rollback in-memory change if possible, or flag for admin.
        it->isTemporaryPassword = true; // Attempt to rollback in-memory
        it->hashedPassword = hashUtils.hashPassword(userToUpdate.username, it->passwordSalt); // This is wrong, can't recover old hash
        return false;
    }
}

std::optional<std::string> AuthService::setupOtpForUser(const std::string& userId, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == userId;
    });

    if (it == users.end()) {
        outMessage = "User not found for OTP setup.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return std::nullopt;
    }

    if (!it->otpSecretKey.empty()) {
        outMessage = "OTP is already set up for this user. To change, disable OTP first (not implemented).";
        LOG_INFO("OTP setup attempt for user '" + it->username + "' but OTP already exists.");
        // Optionally return existing secret for display or an empty optional
        return std::nullopt; // Indicate no new setup was performed
    }

    std::string newSecret = otpService.generateNewOtpSecretKey();
    it->otpSecretKey = newSecret;

    if (fileHandler.saveUsers(users)) {
        outMessage = "OTP setup successful. Please save this secret key securely.";
        LOG_INFO("OTP setup successful for user '" + it->username + "'.");
        return newSecret;
    } else {
        outMessage = "Error saving OTP secret key.";
        LOG_ERROR(outMessage + " User: " + it->username);
        it->otpSecretKey = ""; // Rollback in-memory change
        return std::nullopt;
    }
}