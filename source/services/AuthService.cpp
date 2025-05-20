// src/services/AuthService.cpp
#include "../include/services/AuthService.hpp"
#include "../include/utils/FileHandler.hpp"
#include "../include/utils/HashUtils.hpp"
#include "../include/utils/Logger.hpp" // For LOG_ macros
#include "../include/utils/TimeUtils.hpp"
#include "../include/config/AppConfig.hpp"
#include <algorithm>
#include <ctime>

AuthService::AuthService(std::vector<User>& users_ref, FileHandler& fh_ref, OTPService& otp_ref, HashUtils& hu_ref)
    : users(users_ref), fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {
    // Load users from file when service is initialized
    if (!fileHandler.loadUsers(users)) {
        LOG_ERROR("Failed to load users from file during AuthService initialization");
    } else {
        LOG_INFO("Loaded " + std::to_string(users.size()) + " users from file");
    }
}

bool AuthService::registerUser(const std::string& username, const std::string& password,
                               const std::string& fullName, const std::string& email,
                               const std::string& phoneNumber, UserRole role, std::string& outMessage) {
    // Validate input
    if (username.empty() || password.empty() || fullName.empty() || email.empty()) {
        outMessage = "All fields are required.";
        return false;
    }

    // Check password length
    if (password.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        outMessage = "Password must be at least " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " characters long.";
        return false;
    }

    // Check if username already exists
    auto it = std::find_if(users.begin(), users.end(),
                    [&username](const User& u) { return u.username == username; });
    if (it != users.end()) {
        outMessage = "Username already exists.";
        return false;
    }

    // Create new user
    User newUser;
    newUser.userId = hashUtils.generateUUID();
    newUser.username = username;
    
    // Generate salt and hash password
    std::string salt = hashUtils.generateSalt(16);
    newUser.passwordHash = salt + hashUtils.hashPassword(password, salt);
    
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::NotActivated;
    newUser.isTemporaryPassword = false;

    LOG_INFO("Creating new user '" + username + "' with role: " + User::roleToString(role));

    // Add to users list
    users.push_back(newUser);
    
    if (!fileHandler.saveUsers(users)) {
        users.pop_back(); // Remove the user if save failed
        outMessage = "Failed to save user data. Please try again.";
        return false;
    }
    
    outMessage = "User registered successfully.";
    return true;
}

std::optional<User> AuthService::loginUser(const std::string& username, const std::string& password, std::string& outMessage) {
    // Find user by username
    auto it = std::find_if(users.begin(), users.end(),
                    [&username](const User& u) { return u.username == username; });
    
    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        return std::nullopt;
    }

    // Check if account is active
    if (it->status != AccountStatus::Active) {
        outMessage = "Tai khoan chua duoc kich hoat hoac da bi khoa.";
        return std::nullopt;
    }

    // Verify password
    std::string salt = it->passwordHash.substr(0, 16);
    std::string storedHash = it->passwordHash.substr(16);
    std::string inputHash = hashUtils.hashPassword(password, salt);

    if (inputHash != storedHash) {
        outMessage = "Mat khau khong dung.";
        return std::nullopt;
    }

    outMessage = "Dang nhap thanh cong.";
    return *it; // Return a copy of the user object
}

bool AuthService::changePassword(const std::string& currentUserId, const std::string& oldPassword,
                                   const std::string& newPassword, const std::string& otpCode, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(),
                          [&currentUserId](const User& u) { return u.userId == currentUserId; });

    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        return false;
    }

    // OTP Verification if user has OTP enabled
    if (!it->otpSecretKey.empty()) {
        if (otpCode.empty()) {
            outMessage = "Ban can nhap ma OTP de xac nhan thay doi mat khau.";
            return false;
        }
        if (!otpService.verifyOtp(it->otpSecretKey, otpCode)) {
            outMessage = "Ma OTP khong hop le.";
        return false;
    }
    }

    // Extract salt from the stored password hash
    std::string salt = it->passwordHash.substr(0, 16); // First 16 characters are the salt
    std::string storedHash = it->passwordHash.substr(16); // Rest is the actual hash
    
    // Hash the input password with the salt
    std::string inputHash = hashUtils.hashPassword(oldPassword, salt);
    
    if (storedHash != inputHash) {
        outMessage = "Mat khau hien tai khong chinh xac.";
        return false;
    }

    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        outMessage = "Mat khau moi phai co it nhat " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " ky tu.";
        return false;
    }

    // Generate new salt and hash for the new password
    std::string newSalt = hashUtils.generateSalt(16);
    std::string newHash = hashUtils.hashPassword(newPassword, newSalt);
    it->passwordHash = newSalt + newHash;
    it->isTemporaryPassword = false;
    outMessage = "Mat khau da duoc thay doi thanh cong.";
    return true;
}

std::string AuthService::createAccountWithTemporaryPassword(const std::string& username,
                                                            const std::string& fullName, const std::string& email,
                                                            const std::string& phoneNumber, UserRole role,
                                                            std::string& outMessage) {
    // Validate input
    if (username.empty() || fullName.empty() || email.empty()) {
        outMessage = "Ten dang nhap, ten day du va email la bat buoc.";
        return "";
    }

    // Check if username already exists
    if (std::find_if(users.begin(), users.end(),
                    [&username](const User& u) { return u.username == username; }) != users.end()) {
        outMessage = "Ten dang nhap da ton tai.";
        return "";
    }

    // Generate temporary password
    std::string tempPassword = hashUtils.generateRandomPassword(AppConfig::MIN_PASSWORD_LENGTH);

    // Create new user
    User newUser;
    newUser.userId = hashUtils.generateUUID();
    newUser.username = username;
    
    // Generate salt and hash password
    std::string salt = hashUtils.generateSalt(16);
    newUser.passwordHash = salt + hashUtils.hashPassword(tempPassword, salt);
    
    newUser.fullName = fullName;
    newUser.email = email;
    newUser.phoneNumber = phoneNumber;
    newUser.role = role;
    newUser.status = AccountStatus::NotActivated;
    newUser.isTemporaryPassword = true;

    // Add to users list
    users.push_back(newUser);
    outMessage = "Tai khoan da duoc tao thanh cong voi mat khau tam thoi.";
    return tempPassword;
}

bool AuthService::forceTemporaryPasswordChange(User& userToUpdate, const std::string& newPassword, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(),
                          [&userToUpdate](const User& u) { return u.userId == userToUpdate.userId; });

    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        return false;
    }

    if (!it->isTemporaryPassword) {
        outMessage = "Tai khoan khong co mat khau tam thoi.";
        return false;
    }

    if (newPassword.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        outMessage = "Mat khau moi phai co it nhat " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " ky tu.";
        return false;
    }

    // Generate new salt and hash for the new password
    std::string newSalt = hashUtils.generateSalt(16);
    it->passwordHash = newSalt + hashUtils.hashPassword(newPassword, newSalt);
    it->isTemporaryPassword = false;
    outMessage = "Mat khau da duoc thay doi thanh cong.";
        return true;
}

bool AuthService::updateUser(const User& userToUpdate, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(),
                          [&userToUpdate](const User& u) { return u.userId == userToUpdate.userId; });

    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        return false;
    }

    // Update user fields
    it->fullName = userToUpdate.fullName;
    it->email = userToUpdate.email;
    it->phoneNumber = userToUpdate.phoneNumber;
    it->role = userToUpdate.role;
    it->status = userToUpdate.status;
    it->isTemporaryPassword = userToUpdate.isTemporaryPassword;

    outMessage = "Tai khoan da duoc cap nhat thanh cong.";
    return true;
}

std::optional<std::string> AuthService::setupOtpForUser(const std::string& userId, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(), [&](User& u) {
        return u.userId == userId;
    });

    if (it == users.end()) {
        outMessage = "Khong tim thay tai khoan.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return std::nullopt;
    }

    if (!it->otpSecretKey.empty()) {
        outMessage = "OTP da duoc thiet lap cho tai khoan nay. De thay doi, vui long tat OTP (chua thuc hien).";
        LOG_INFO("OTP thiet lap thu cho tai khoan '" + it->username + "' nhung OTP da ton tai.");
        // Optionally return existing secret for display or an empty optional
        return std::nullopt; // Indicate no new setup was performed
    }

    std::string newSecret = otpService.generateNewOtpSecretKey();
    it->otpSecretKey = newSecret;

    if (fileHandler.saveUsers(users)) {
        outMessage = "OTP thiet lap thanh cong. Vui long luu lai OTP.";
        LOG_INFO("OTP thiet lap thanh cong cho tai khoan '" + it->username + "'.");
        return newSecret;
    } else {
        outMessage = "Loi khi luu OTP.";
        LOG_ERROR(outMessage + " Tai khoan: " + it->username);
        it->otpSecretKey = ""; // Rollback in-memory change
        return std::nullopt;
    }
}

bool AuthService::activateAccount(const std::string& username, std::string& outMessage) {
    auto it = std::find_if(users.begin(), users.end(),
                          [&username](const User& u) { return u.username == username; });

    if (it == users.end()) {
        outMessage = "That bai: Khong tim thay tai khoan.";
        return false;
    }

    if (it->status == AccountStatus::Active) {
        outMessage = "That bai: Tai khoan da duoc kich hoat truoc do.";
        return false;
    }

    it->status = AccountStatus::Active;
    if (fileHandler.saveUsers(users)) {
        outMessage = "Thanh cong: Kich hoat tai khoan thanh cong.";
        return true;
    } else {
        outMessage = "That bai: Loi khi luu trang thai tai khoan.";
        return false;
    }
}