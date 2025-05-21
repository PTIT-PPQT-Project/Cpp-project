// src/services/WalletService.cpp
#include "services/WalletService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeUtils.hpp"    
#include "Config.h"                
#include <algorithm>
#include <ctime>
#include <iomanip>                 
#include <sstream>              

WalletService::WalletService(std::vector<User>& u_ref, std::vector<Wallet>& w_ref, 
                             std::vector<Transaction>& t_ref, FileHandler& fh_ref, 
                             OTPService& otp_ref, HashUtils& hu_ref)
    : users(u_ref), wallets(w_ref), transactions(t_ref), 
      fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

bool WalletService::createWalletForUser(const std::string& userId, std::string& outMessage) {
    auto user_it = std::find_if(users.cbegin(), users.cend(), [&](const User& u){ return u.userId == userId; });
    if (user_it == users.cend()){
        outMessage = "Nguoi dung khong tim thay, khong the tao vi.";
        return false;
    }

    auto wallet_exists = std::find_if(wallets.cbegin(), wallets.cend(), 
                                      [&](const Wallet& w) { return w.userId == userId; });
    if (wallet_exists != wallets.cend()) {
        outMessage = "Vi da ton tai cho nguoi dung " + user_it->username + ".";
        return true; // Not an error, wallet exists.
    }

    Wallet newWallet;
    newWallet.walletId = "WLT-" + hashUtils.generateUUID().substr(0, 12); // Generate an unique wallet ID
    newWallet.userId = userId;
    newWallet.balance = AppConfig::DEFAULT_INITIAL_WALLET_BALANCE; // Use config
    newWallet.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newWallet.lastUpdateTimestamp = newWallet.creationTimestamp;

    wallets.push_back(newWallet);
    if (fileHandler.saveWallets(wallets)) {
        outMessage = "Da tao thanh cong vi moi cho nguoi dung " + user_it->username + ". ID vi: " + newWallet.walletId;
        return true;
    } else {
        outMessage = "Loi khi luu du lieu vi moi.";
        wallets.pop_back(); // Rollback
        return false;
    }
}

std::optional<Wallet> WalletService::getWalletByUserId(const std::string& userId) const {
    auto it = std::find_if(wallets.cbegin(), wallets.cend(), 
                           [&](const Wallet& w) { return w.userId == userId; });
    if (it != wallets.cend()) {
        return *it;
    }
    return std::nullopt;
}

std::optional<Wallet> WalletService::getWalletByWalletId(const std::string& walletId) const {
     auto it = std::find_if(wallets.cbegin(), wallets.cend(), 
                            [&](const Wallet& w) { return w.walletId == walletId; });
    if (it != wallets.cend()) {
        return *it;
    }
    return std::nullopt;
}

std::optional<Wallet> WalletService::getWalletByUsername(const std::string& username) const {
    // First find the user by username
    auto user_it = std::find_if(users.cbegin(), users.cend(),
                               [&](const User& u) { return u.username == username; });
    if (user_it == users.cend()) {
        return std::nullopt;
    }
    
    // Then find the wallet for that user
    auto wallet_it = std::find_if(wallets.cbegin(), wallets.cend(),
                                 [&](const Wallet& w) { return w.userId == user_it->userId; });
    if (wallet_it != wallets.cend()) {
        return *wallet_it;
    }
    return std::nullopt;
}

bool WalletService::transferPoints(const std::string& senderUserId, const std::string& senderWalletId,
                                   const std::string& receiverWalletId, double amount,
                                   const std::string& otpCode, std::string& outMessage) {
    if (amount <= 0) {
        outMessage = "So tien chuyen phai la so duong.";
        LOG_WARNING("Chuyen tien that bai: " + outMessage + " So tien: " + std::to_string(amount));
        return false;
    }
    if (senderWalletId == receiverWalletId) {
        outMessage = "Khong the chuyen diem den cung mot vi.";
        LOG_WARNING("Chuyen tien that bai: ID vi cua nguoi gui va nguoi nhan giong nhau: " + senderWalletId);
        return false;
    }

    auto senderUserIt = std::find_if(users.cbegin(), users.cend(), 
                                     [&](const User& u) { return u.userId == senderUserId; });
    if (senderUserIt == users.cend()) {
        outMessage = "Nguoi gui khong tim thay.";
        LOG_ERROR("Chuyen tien that bai: " + outMessage + " ID nguoi gui: " + senderUserId);
        return false;
    }

    // OTP Verification if sender has OTP enabled
    if (!senderUserIt->otpSecretKey.empty()) {
        if (otpCode.empty()) {
            outMessage = "Ma OTP la bat buoc cho chuyen tien.";
            LOG_WARNING("Chuyen tien that bai: nguoi dung '" + senderUserIt->username + "': Thieu ma OTP.");
            return false;
        }
        if (!otpService.verifyOtp(senderUserIt->otpSecretKey, otpCode)) {
            outMessage = "Ma OTP khong hop le.";
            LOG_WARNING("Chuyen tien that bai: User '" + senderUserIt->username + "': Ma OTP khong hop le.");
            return false;
        }
    }

    Wallet* pSenderWallet = nullptr;
    Wallet* pReceiverWallet = nullptr;
    // Find wallets by reference to modify them directly in the g_wallets vector
    for (Wallet& w : wallets) { // Use non-const reference
        if (w.walletId == senderWalletId) pSenderWallet = &w;
        if (w.walletId == receiverWalletId) pReceiverWallet = &w;
    }

    if (!pSenderWallet) {
        outMessage = "Vi cua nguoi gui (ID: " + senderWalletId + ") khong tim thay.";
        LOG_WARNING("Chuyen tien that bai: " + outMessage);
        return false;
    }
    if (pSenderWallet->userId != senderUserId) {
        outMessage = "Vi cua nguoi gui khong phai cua ban.";
        LOG_ERROR("Chuyen tien that bai: User '" + senderUserIt->username + "' (ID: " + senderUserId +
                  ") dang su dung vi '" + senderWalletId + "' khong phai cua ban.");
        return false;
    }
    if (!pReceiverWallet) {
        outMessage = "Vi nguoi nhan (ID: " + receiverWalletId + ") khong tim thay.";
        LOG_WARNING("Chuyen tien that bai: " + outMessage);
        return false;
    }

    Transaction tx;
    tx.transactionId = "TXN-" + hashUtils.generateUUID().substr(0,12);
    tx.sourceWalletId = senderWalletId;
    tx.targetWalletId = receiverWalletId;
    tx.amount = amount;
    tx.timestamp = TimeUtils::getCurrentTimestamp();
    std::stringstream desc_ss;
    desc_ss << "Chuyen tu " << senderUserIt->username << " (vi: " << senderWalletId 
            << ") den vi: " << receiverWalletId;
    tx.description = desc_ss.str();

    if (pSenderWallet->balance < amount) {
        outMessage = "So du khong du. Hien co: " + std::to_string(pSenderWallet->balance) + ", can chuyen: " + std::to_string(amount);
        tx.status = TransactionStatus::Failed;
        transactions.push_back(tx);
        if(!fileHandler.saveTransactions(transactions)) {
             LOG_ERROR("Failed to save transaction log for failed (insufficient funds) TxID: " + tx.transactionId);
        }
        LOG_WARNING("Chuyen tien that bai cho user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }

    // Perform transfer (in-memory first)
    double originalSenderBalance = pSenderWallet->balance; // For potential rollback
    double originalReceiverBalance = pReceiverWallet->balance; // For potential rollback

    pSenderWallet->balance -= amount;
    pReceiverWallet->balance += amount;
    time_t updateTime = TimeUtils::getCurrentTimestamp(); // Use a single timestamp for consistency
    pSenderWallet->lastUpdateTimestamp = updateTime;
    pReceiverWallet->lastUpdateTimestamp = updateTime;

    // Attempt to save. Atomicity is a concern with multiple file writes.
    if (fileHandler.saveWallets(wallets)) { // Step 1: Save updated wallets
        tx.status = TransactionStatus::Completed;
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) { // Step 2: Save successful transaction
            outMessage = "Chuyen diem thanh cong!";
            LOG_INFO("Chuyen diem thanh cong! TxID: " + tx.transactionId);
            return true;
        } else {
            // Wallets saved, but transaction log failed. This is a critical inconsistency.
            outMessage = "Chuyen diem thanh cong nhung khong the luu lai lich su giao dich. Vui long lien he ho tro.";
            LOG_ERROR("CRITICAL INCONSISTENCY: Wallets updated for TxID " + tx.transactionId +
                      " but transaction log FAILED to save. Sender new balance: " + std::to_string(pSenderWallet->balance) +
                      ", Receiver new balance: " + std::to_string(pReceiverWallet->balance));
            return true; // Indicate balance change happened but log failed
        }
    } else {
        // Failed to save wallets. Rollback in-memory changes.
        pSenderWallet->balance = originalSenderBalance;
        pReceiverWallet->balance = originalReceiverBalance;

        outMessage = "Khong the luu thay doi vi. Giao dich da duoc huy.";
        tx.status = TransactionStatus::Failed;
        transactions.push_back(tx); // Log the system error that prevented the transfer
        if(!fileHandler.saveTransactions(transactions)){
            LOG_ERROR("Failed to save transaction log for system error rollback (TxID: " + tx.transactionId + ")");
        }
        LOG_ERROR("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }
}

std::vector<Transaction> WalletService::getTransactionHistory(const std::string& walletId) const {
    std::vector<Transaction> history;
    for (const auto& tx : transactions) {
        if (tx.sourceWalletId == walletId || tx.targetWalletId == walletId) {
            history.push_back(tx);
        }
    }
    // Sort by timestamp, newest first
    std::sort(history.begin(), history.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp > b.timestamp;
    });
    LOG_DEBUG("Retrieved " + std::to_string(history.size()) + " transactions for Wallet ID: " + walletId);
    return history;
}

bool WalletService::depositPoints(const std::string& targetWalletId, double amount, 
                                  const std::string& description, const std::string& initiatedByUserId,
                                  std::string& outMessage, 
                                  const std::string& sourceWalletId) {
    if (amount <= 0) {
        outMessage = "Deposit amount must be positive.";
        LOG_WARNING("Deposit attempt failed: " + outMessage + " Amount: " + std::to_string(amount));
        return false;
    }

    Wallet* pTargetWallet = nullptr;
    for (auto& w : wallets) { // Non-const ref for modification
        if (w.walletId == targetWalletId) pTargetWallet = &w;
    }

    if (!pTargetWallet) {
        outMessage = "Target wallet (ID: " + targetWalletId + ") for deposit not found.";
        LOG_WARNING("Deposit failed: " + outMessage);
        return false;
    }

    Transaction tx;
    tx.transactionId = "DEP-" + hashUtils.generateUUID().substr(0,12);
    tx.sourceWalletId = sourceWalletId;
    tx.targetWalletId = targetWalletId;
    tx.amount = amount;
    tx.timestamp = TimeUtils::getCurrentTimestamp();
    tx.description = description + " (Initiated by: " + initiatedByUserId + ")";
    tx.status = TransactionStatus::Completed;

    double originalTargetBalance = pTargetWallet->balance; // For potential rollback

    // Update balance
    pTargetWallet->balance += amount;
    pTargetWallet->lastUpdateTimestamp = TimeUtils::getCurrentTimestamp();

    // Save updated wallets
    if (fileHandler.saveWallets(wallets)) {
        // Save transaction record
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) {
            outMessage = "Deposit successful. New balance: " + std::to_string(pTargetWallet->balance);
            LOG_INFO("Deposit successful for wallet " + targetWalletId + 
                     ". Amount: " + std::to_string(amount) + 
                     ", New balance: " + std::to_string(pTargetWallet->balance));
            return true;
        } else {
            // Rollback wallet balance if transaction save fails
            pTargetWallet->balance = originalTargetBalance;
            pTargetWallet->lastUpdateTimestamp = TimeUtils::getCurrentTimestamp();
            fileHandler.saveWallets(wallets); // Save the rollback
            outMessage = "Deposit processed but failed to record transaction. Balance has been restored.";
            LOG_ERROR("Deposit failed to save transaction record for wallet " + targetWalletId);
            return false;
        }
    } else {
        // Rollback in-memory changes
        pTargetWallet->balance = originalTargetBalance;
        outMessage = "Failed to save wallet updates. Deposit has been rolled back.";
        LOG_ERROR("Deposit failed to save wallet updates for wallet " + targetWalletId);
        return false;
    }
}