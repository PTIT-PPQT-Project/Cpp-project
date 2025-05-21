// src/services/AdminService.cpp
#include "services/AdminService.hpp"
#include "utils/Logger.hpp"
#include "Config.h" // For AppConfig::MASTER_WALLET_ID if used
#include "utils/InputValidator.hpp"

AdminService::AdminService(std::vector<User>& u_ref, AuthService& as_ref,
                           UserService& us_ref, WalletService& ws_ref)
    : users(u_ref), authService(as_ref), userService(us_ref), walletService(ws_ref) {}

std::vector<User> AdminService::listAllUsers() const {
    LOG_INFO("Admin len danh sach tat ca nguoi dung.");
    return users; // Returns a copy. For performance on large sets, consider const ref or specific DTOs.
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
                LOG_ERROR("Admin da tao tai khoan cho '" + username + "' nhung that bai khi tao vi: " + walletMsg);
                // User created, but wallet failed. This is a partial success/failure state.
                // outMessage might need to reflect this.
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
    return false;
}

bool AdminService::adminUpdateUserProfile(const std::string& adminUserId, const std::string& targetUserId,
                                          const std::string& newFullName, const std::string& newEmail,
                                          const std::string& newPhoneNumber, AccountStatus newStatus,
                                          const std::string& targetUserOtpCode,
                                          std::string& outMessage) {
    LOG_INFO("Admin '" + adminUserId + "' dang cap nhat thong tin nguoi dung co ID '" + targetUserId + "'.");

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Khong tim thay tai khoan can cap nhat.";
        return false;
    }
    User targetUser = targetUserOpt.value();
    // OTP Verification if target user has OTP enabled
    if (!targetUser.otpSecretKey.empty()) {
        if (targetUserOtpCode.empty()) {
            outMessage = "Can nhap ma OTP cua nguoi dung de xac nhan thay doi.";
            return false;
        }
        if (!authService.getOtpService().verifyOtp(targetUser.otpSecretKey, targetUserOtpCode)) {
            outMessage = "Ma OTP cua nguoi dung khong hop le.";
            return false;
        }
    }

    // Now, actually find the user in the main vector to update
    auto it_target = std::find_if(users.begin(), users.end(), [&](User& u){ return u.userId == targetUserId; });
    if (it_target == users.end()){ 
        outMessage = "Loi he thong: Tai khoan nguoi dung tim thay ban dau nhung khong tim thay trong danh sach cap nhat.";
        LOG_ERROR(outMessage + " ID nguoi dung: " + targetUserId);
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
            outMessage = "Dinh dang email moi khong hop le.";
            LOG_WARNING("Admin cap nhat thong tin nguoi dung '" + it_target->username + "' that bai: " + outMessage);
            return false;
        }
        auto email_exists = std::find_if(users.cbegin(), users.cend(), [&](const User& u) {
            return u.email == newEmail && u.userId != targetUserId;
        });
        if (email_exists != users.cend()) {
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
        return true; // Or false if "no change" is an issue
    }

    if (authService.getFileHandler().saveUsers(users)) { // Use fileHandler from one of the services, e.g. authService
        outMessage = "Admin da cap nhat thong tin nguoi dung " + it_target->username + " thanh cong.";
        LOG_INFO(outMessage);
        return true;
    } else {
        outMessage = "Loi khi luu thong tin nguoi dung cap nhat boi admin.";
        LOG_ERROR(outMessage + " Tai khoan cua nguoi dung: " + it_target->username);
        // Consider rollback for in-memory changes.
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

    auto targetUserOpt = userService.getUserProfile(targetUserId);
    if (!targetUserOpt) {
        outMessage = "Tai khoan cua nguoi dung khong tim thay. Vui long kiem tra lai.";
        LOG_WARNING("Admin chuyen khoan loi: " + outMessage + " Toi tai khoan cua nguoi dung " + targetUserId);
        return false;
    }
    auto targetWalletOpt = walletService.getWalletByUserId(targetUserId);
    if (!targetWalletOpt) {
        outMessage = "Tai khoan cua nguoi dung khong tim thay. Vui long kiem tra lai.";
        LOG_WARNING("Admin chuyen khoan loi: " + outMessage + " Toi tai khoan cua nguoi dung " + targetUserOpt.value().username);
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

    std::string description = "Admin chuyen khoan (" + adminUserId + "): " + "voi ly do:" + reason;
    // Using MASTER_WALLET_ID or a generic system source ID for deposits from admin
    return walletService.depositPoints(targetWalletOpt.value().walletId, amount, description, adminUserId, 
                                       outMessage, AppConfig::MASTER_WALLET_ID);
}