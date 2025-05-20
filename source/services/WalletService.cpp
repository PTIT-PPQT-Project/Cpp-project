// src/services/WalletService.cpp
#include "services/WalletService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeUtils.hpp"     // For TimeUtils::getCurrentTimestamp
#include "Config.h"                // For AppConfig constants
#include <algorithm>
#include <ctime>
#include <iomanip>                 // For std::fixed, std::setprecision in logging
#include <sstream>                 // For std::stringstream in tx description

WalletService::WalletService(std::vector<User>& u_ref, std::vector<Wallet>& w_ref, 
                             std::vector<Transaction>& t_ref, FileHandler& fh_ref, 
                             OTPService& otp_ref, HashUtils& hu_ref)
    : users(u_ref), wallets(w_ref), transactions(t_ref), 
      fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

bool WalletService::createWalletForUser(const std::string& userId, std::string& outMessage) {
    auto user_it = std::find_if(users.cbegin(), users.cend(), [&](const User& u){ return u.userId == userId; });
    if (user_it == users.cend()){
        outMessage = "User not found, cannot create wallet.";
        return false;
    }

    auto wallet_exists = std::find_if(wallets.cbegin(), wallets.cend(), 
                                      [&](const Wallet& w) { return w.userId == userId; });
    if (wallet_exists != wallets.cend()) {
        outMessage = "Wallet already exists for user " + user_it->username + ".";
        return true; // Not an error, wallet exists.
    }

    Wallet newWallet;
    newWallet.walletId = "WLT-" + hashUtils.generateUUID().substr(0, 12); // Generate a unique wallet ID
    newWallet.userId = userId;
    newWallet.balance = AppConfig::DEFAULT_INITIAL_WALLET_BALANCE; // Use config
    newWallet.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newWallet.lastUpdateTimestamp = newWallet.creationTimestamp;

    wallets.push_back(newWallet);
    if (fileHandler.saveWallets(wallets)) {
        outMessage = "New wallet created successfully for user " + user_it->username + ". Wallet ID: " + newWallet.walletId;
        return true;
    } else {
        outMessage = "Error saving new wallet data.";
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
    // LOG_DEBUG("Wallet not found for User ID: " + userId); // Can be noisy, use DEBUG if needed
    return std::nullopt;
}

std::optional<Wallet> WalletService::getWalletByWalletId(const std::string& walletId) const {
     auto it = std::find_if(wallets.cbegin(), wallets.cend(), 
                            [&](const Wallet& w) { return w.walletId == walletId; });
    if (it != wallets.cend()) {
        return *it;
    }
    // LOG_DEBUG("Wallet not found for Wallet ID: " + walletId);
    return std::nullopt;
}

bool WalletService::transferPoints(const std::string& senderUserId, const std::string& senderWalletId,
                                   const std::string& receiverWalletId, double amount,
                                   const std::string& otpCode, std::string& outMessage) {
    if (amount <= 0) {
        outMessage = "Transfer amount must be positive.";
        LOG_WARNING("Transfer attempt failed: " + outMessage + " Amount: " + std::to_string(amount));
        return false;
    }
    if (senderWalletId == receiverWalletId) {
        outMessage = "Cannot transfer points to the same wallet.";
        LOG_WARNING("Transfer attempt failed: Sender and Receiver wallet IDs are identical: " + senderWalletId);
        return false;
    }

    auto senderUserIt = std::find_if(users.cbegin(), users.cend(), 
                                     [&](const User& u) { return u.userId == senderUserId; });
    if (senderUserIt == users.cend()) {
        outMessage = "Sender user not found."; // This should ideally not happen if senderUserId is from a logged-in session
        LOG_ERROR("Transfer critical error: " + outMessage + " User ID: " + senderUserId);
        return false;
    }

    // OTP Verification if sender has OTP enabled
    if (!senderUserIt->otpSecretKey.empty()) {
        if (otpCode.empty()) {
            outMessage = "OTP is required for this transfer as you have it enabled.";
            LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': Missing OTP.");
            return false;
        }
        if (!otpService.verifyOtp(senderUserIt->otpSecretKey, otpCode)) {
            outMessage = "Invalid OTP code.";
            LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': Invalid OTP.");
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
        outMessage = "Sender wallet (ID: " + senderWalletId + ") not found.";
        LOG_WARNING("Transfer failed: " + outMessage);
        return false;
    }
    if (pSenderWallet->userId != senderUserId) {
        outMessage = "Sender wallet does not belong to the authenticated user. Operation denied.";
        LOG_ERROR("Transfer security alert: User '" + senderUserIt->username + "' (ID: " + senderUserId +
                  ") attempted to use wallet '" + senderWalletId + "' not belonging to them.");
        return false;
    }
    if (!pReceiverWallet) {
        outMessage = "Receiver wallet (ID: " + receiverWalletId + ") not found.";
        LOG_WARNING("Transfer failed: " + outMessage);
        return false;
    }

    Transaction tx;
    tx.transactionId = "TXN-" + hashUtils.generateUUID().substr(0,12);
    tx.sourceWalletId = senderWalletId;
    tx.targetWalletId = receiverWalletId;
    tx.amount = amount;
    tx.timestamp = TimeUtils::getCurrentTimestamp();
    std::stringstream desc_ss;
    desc_ss << "Transfer from " << senderUserIt->username << " (Wallet: " << senderWalletId 
            << ") to Wallet: " << receiverWalletId;
    tx.description = desc_ss.str();

    if (pSenderWallet->balance < amount) {
        outMessage = "Insufficient balance. Available: " + std::to_string(pSenderWallet->balance) + ", Required: " + std::to_string(amount);
        tx.status = TransactionStatus::Failed;
        transactions.push_back(tx);
        if(!fileHandler.saveTransactions(transactions)) {
             LOG_ERROR("Failed to save transaction log for failed (insufficient funds) TxID: " + tx.transactionId);
        }
        LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
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
            outMessage = "Points transferred successfully!";
            LOG_INFO(outMessage + " TxID: " + tx.transactionId + ", Amount: " + std::to_string(amount) +
                     " from " + senderWalletId + " to " + receiverWalletId);
            return true;
        } else {
            // Wallets saved, but transaction log failed. This is a critical inconsistency.
            outMessage = "Transfer processed and wallet balances updated, but failed to record transaction log. Please contact support with TxID: " + tx.transactionId;
            LOG_ERROR("CRITICAL INCONSISTENCY: Wallets updated for TxID " + tx.transactionId +
                      " but transaction log FAILED to save. Sender new balance: " + std::to_string(pSenderWallet->balance) +
                      ", Receiver new balance: " + std::to_string(pReceiverWallet->balance));
            // At this point, data is inconsistent. A more robust system would use a 2-phase commit or journaling.
            return true; // Indicate balance change happened but log failed. Or return false.
        }
    } else {
        // Failed to save wallets. Rollback in-memory changes.
        pSenderWallet->balance = originalSenderBalance;
        pReceiverWallet->balance = originalReceiverBalance;
        // Revert timestamps (or don't update them if save fails)
        // pSenderWallet->lastUpdateTimestamp = originalSenderLastUpdate; // Need to store this too for perfect rollback
        // pReceiverWallet->lastUpdateTimestamp = originalReceiverLastUpdate;

        outMessage = "Failed to save wallet updates. Transfer has been rolled back.";
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