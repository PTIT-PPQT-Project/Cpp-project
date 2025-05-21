// src/services/AdminService.cpp
#include "services/AdminService.hpp"
#include "utils/Logger.hpp"
#include "Config.h"
#include "utils/InputValidator.hpp"

AdminService::AdminService(std::vector<User>& u_ref, AuthService& as_ref,
                           UserService& us_ref, WalletService& ws_ref)
    : users(u_ref), authService(as_ref), userService(us_ref), walletService(ws_ref) {}

const std::vector<User>& AdminService::listAllUsers() const {
    LOG_INFO("Admin len danh sach tat ca nguoi dung.");
    return users; // Return const reference instead of copying
}

bool AdminService::adminCreateUserAccount(const std::string& username, const std::string& fullName,
                                          const std::string& email, const std::string& phoneNumber,
                                          UserRole role, std::string& outCreatedTempPassword,
                                          std::string& outMessage) {
    // Policy: Admin cannot create another Admin user through this simple function for safety.
    if (role == UserRole::AdminUser) {
        outMessage = "Khong the tao tai khoan Admin su dung chuc nang nay vi ly do an toan.";
        LOG_WARNING("Admin dang thu tao mot tai khoan Admin khac cho '" + username + "' bi chan.");
        return false;
    }
    
    // Create account with temporary password
    outCreatedTempPassword = authService.createAccountWithTemporaryPassword(
                                username, fullName, email, phoneNumber, role, outMessage);

    if (outCreatedTempPassword.empty()) {
        return false; // Account creation failed
    }
    
    // Find the new user ID - efficient search with early exit
    std::string newUserId;
    auto it = std::find_if(users.begin(), users.end(), 
                           [&username](const User& u) { return u.username == username; });
    
    if (it != users.end()) {
        newUserId = it->userId;
        
        // Auto-create wallet for the new user
        std::string walletMsg;
        if (!walletService.createWalletForUser(newUserId, walletMsg)) {
            LOG_ERROR("Admin da tao tai khoan cho '" + username + 
                      "' nhung that bai khi tao vi: " + walletMsg);
            outMessage += " Tai khoan da duoc tao nhung tao vi that bai: " + walletMsg;
        } else {
            LOG_INFO("Vi da duoc tao cho tai khoan moi '" + username + "' boi admin.");
        }
    } else {
        LOG_ERROR("Khong the tim thay tai khoan '" + username + "' de tao vi boi admin.");
        outMessage += " Tai khoan da duoc tao nhung khong tim thay nguoi dung de tao vi.";
    }
    
    return true;
}

bool AdminService::adminUpdateUserProfile(const std::string& adminUserId, const std::string& targetUserId,
                                          const std::string& newFullName, const std::string& newEmail,
                                          const std::string& newPhoneNumber, AccountStatus newStatus,
                                          const std::string& targetUserOtpCode,
                                          std::string& outMessage) {
    LOG_INFO("Admin '" + adminUserId + "' dang cap nhat thong tin nguoi dung co ID '" + targetUserId + "'.");

    // Find the user directly in the main vector instead of calling getUserProfile first
    auto it_target = std::find_if(users.begin(), users.end(), 
                                 [&targetUserId](const User& u) { return u.userId == targetUserId; });
    
    if (it_target == users.end()) {
        outMessage = "Khong tim thay tai khoan can cap nhat.";
        return false;
    }
    
    // OTP Verification if target user has OTP enabled
    if (!it_target->otpSecretKey.empty()) {
        if (targetUserOtpCode.empty()) {
            outMessage = "Can nhap ma OTP cua nguoi dung de xac nhan thay doi.";
            return false;
        }
        if (!authService.getOtpService().verifyOtp(it_target->otpSecretKey, targetUserOtpCode)) {
            outMessage = "Ma OTP cua nguoi dung khong hop le.";
            return false;
        }
    }

    // Track changes for efficient saving
    bool changed = false;
    
    // Update fields - only modified fields trigger saves
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
        // Validate email format
        if (!InputValidator::isValidEmail(newEmail)) {
            outMessage = "Dinh dang email moi khong hop le.";
            LOG_WARNING("Admin cap nhat thong tin nguoi dung '" + it_target->username + "' that bai: " + outMessage);
            return false;
        }
        
        // Check if email is already in use by another account (efficient with early exit)
        bool emailExists = std::any_of(users.begin(), users.end(), 
            [&newEmail, &targetUserId](const User& u) {
                return u.email == newEmail && u.userId != targetUserId;
            });
            
        if (emailExists) {
            outMessage = "Email moi da duoc su dung boi mot tai khoan khac.";
            LOG_WARNING("Admin cap nhat thong tin nguoi dung '" + it_target->username + "' that bai: " + outMessage);
            return false;
        }
        
        it_target->email = newEmail;
        changed = true;
    }

    if (!changed) {
        outMessage = "Khong co thay doi nao duoc cung cap cho thong tin nguoi dung.";
        LOG_INFO("Admin cap nhat thong tin nguoi dung '" + it_target->username + "': Khong co thay doi nao.");
        return true; // Success but no changes
    }

    // Only save if changes were made
    if (authService.getFileHandler().saveUsers(users)) {
        outMessage = "Admin da cap nhat thong tin nguoi dung " + it_target->username + " thanh cong.";
        LOG_INFO(outMessage);
        return true;
    } else {
        outMessage = "Loi khi luu thong tin nguoi dung cap nhat boi admin.";
        LOG_ERROR(outMessage + " Tai khoan cua nguoi dung: " + it_target->username);
        return false;
    }
}

bool AdminService::adminActivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin dang kich hoat tai khoan cua nguoi dung co ID '" + targetUserId + "'.");
    return userService.activateUserAccount(targetUserId, outMessage);
}

bool AdminService::adminDeactivateUser(const std::string& targetUserId, std::string& outMessage) {
    LOG_INFO("Admin dang khoa tai khoan cua nguoi dung co ID '" + targetUserId + "'.");
    return userService.deactivateUserAccount(targetUserId, outMessage);
}

bool AdminService::adminDepositToUserWallet(const std::string& adminUserId, const std::string& targetUserId,
                                            double amount, const std::string& reason, std::string& outMessage) {
    LOG_INFO("Admin '" + adminUserId + "' dang chuyen khoan " + std::to_string(amount) + 
             " cho tai khoan cua nguoi dung '" + targetUserId + "' voi ly do: " + reason);

    // Get wallet directly instead of getting user profile first
    auto targetWalletOpt = walletService.getWalletByUserId(targetUserId);
    
    if (!targetWalletOpt) {
        // Check if user exists to provide better error message
        auto userExists = std::any_of(users.begin(), users.end(),
                                     [&targetUserId](const User& u) { return u.userId == targetUserId; });
        
        if (!userExists) {
            outMessage = "Tai khoan cua nguoi dung khong tim thay. Vui long kiem tra lai.";
        } else {
            outMessage = "Nguoi dung khong co vi. Vui long tao vi truoc.";
        }
        
        LOG_WARNING("Admin chuyen khoan loi: " + outMessage + " Toi tai khoan cua nguoi dung " + targetUserId);
        return false;
    }

    std::string description = "Admin chuyen khoan (" + adminUserId + "): " + "voi ly do:" + reason;
    
    // Use std::move to avoid copying the description string
    return walletService.depositPoints(targetWalletOpt.value().walletId, amount, 
                                      std::move(description), adminUserId, 
                                      outMessage, AppConfig::MASTER_WALLET_ID);
}