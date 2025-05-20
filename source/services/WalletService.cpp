// src/services/WalletService.cpp
#include "services/WalletService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp" // For generating transaction IDs
#include "utils/Logger.hpp"
#include "Config.h" // For AppConfig::DEFAULT_INITIAL_WALLET_BALANCE, MASTER_WALLET_ID etc.
#include "utils/TimeUtils.hpp"  // Add missing include for TimeUtils
#include <algorithm>
#include <ctime>
#include <iomanip> // for std::fixed, std::setprecision for logging amounts


WalletService::WalletService(std::vector<User>& u_ref, std::vector<Wallet>& w_ref, 
                             std::vector<Transaction>& t_ref, FileHandler& fh_ref, 
                             OTPService& otp_ref, HashUtils& hu_ref)
    : users(u_ref), wallets(w_ref), transactions(t_ref), 
      fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

bool WalletService::createWalletForUser(const std::string& userId, std::string& outMessage) {
    auto user_it = std::find_if(users.cbegin(), users.cend(), [&](const User& u){ return u.userId == userId; });
    if (user_it == users.cend()){
        outMessage = "User not found, cannot create wallet.";
        LOG_WARNING(outMessage + " User ID: " + userId);
        return false;
    }

    auto wallet_exists = std::find_if(wallets.cbegin(), wallets.cend(), 
                                      [&](const Wallet& w) { return w.userId == userId; });
    if (wallet_exists != wallets.cend()) {
        outMessage = "Wallet already exists for this user.";
        LOG_INFO("Attempted to create wallet for user '" + user_it->username + "' but wallet already exists.");
        return true; // Not an error, wallet exists
    }

    Wallet newWallet;
    newWallet.walletId = "WLT-" + hashUtils.generateUUID().substr(0, 12); // Example wallet ID
    newWallet.userId = userId;
    newWallet.balance = AppConfig::DEFAULT_INITIAL_WALLET_BALANCE;
    newWallet.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newWallet.lastUpdateTimestamp = newWallet.creationTimestamp;

    wallets.push_back(newWallet);
    if (fileHandler.saveWallets(wallets)) {
        outMessage = "New wallet created successfully for user " + user_it->username + ".";
        LOG_INFO(outMessage + " Wallet ID: " + newWallet.walletId);
        return true;
    } else {
        outMessage = "Error saving new wallet data.";
        LOG_ERROR(outMessage + " User: " + user_it->username);
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

bool WalletService::transferPoints(const std::string& senderUserId, const std::string& senderWalletId,
                                   const std::string& receiverWalletId, double amount,
                                   const std::string& otpCode, std::string& outMessage) {
    if (amount <= 0) {
        outMessage = "Transfer amount must be positive.";
        LOG_WARNING("Transfer attempt failed: " + outMessage);
        return false;
    }
    if (senderWalletId == receiverWalletId) {
        outMessage = "Cannot transfer points to the same wallet.";
        LOG_WARNING("Transfer attempt failed: " + outMessage);
        return false;
    }

    auto senderUserIt = std::find_if(users.cbegin(), users.cend(), 
                                     [&](const User& u) { return u.userId == senderUserId; });
    if (senderUserIt == users.cend()) {
        outMessage = "Sender user not found.";
        LOG_ERROR("Transfer critical error: " + outMessage + " User ID: " + senderUserId);
        return false; // Critical error
    }

    if (!senderUserIt->otpSecretKey.empty()) {
        if (otpCode.empty()) {
            outMessage = "OTP is required for this transfer.";
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
    for (auto& w : wallets) {
        if (w.walletId == senderWalletId) pSenderWallet = &w;
        if (w.walletId == receiverWalletId) pReceiverWallet = &w;
    }

    if (!pSenderWallet) {
        outMessage = "Sender wallet not found.";
        LOG_WARNING("Transfer failed: " + outMessage + " Wallet ID: " + senderWalletId);
        return false;
    }
    if (pSenderWallet->userId != senderUserId) {
        outMessage = "Sender wallet does not belong to the authenticated user.";
        LOG_ERROR("Transfer security alert: " + outMessage + " User: " + senderUserId + ", Wallet: " + senderWalletId);
        return false; // Security issue
    }
    if (!pReceiverWallet) {
        outMessage = "Receiver wallet not found.";
        LOG_WARNING("Transfer failed: " + outMessage + " Wallet ID: " + receiverWalletId);
        return false;
    }

    Transaction tx;
    tx.transactionId = "TXN-" + hashUtils.generateUUID().substr(0,12);
    tx.senderWalletId = senderWalletId;
    tx.receiverWalletId = receiverWalletId;
    tx.amountTransferred = amount;
    tx.transactionTimestamp = TimeUtils::getCurrentTimestamp();
    std::stringstream desc_ss;
    desc_ss << "Transfer from " << senderUserIt->username << " to wallet " << receiverWalletId;
    tx.description = desc_ss.str();


    if (pSenderWallet->balance < amount) {
        outMessage = "Insufficient balance in sender wallet.";
        tx.status = TransactionStatus::FailedInsufficientFunds;
        transactions.push_back(tx);
        fileHandler.saveTransactions(transactions); // Log failed transaction
        LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }

    // Perform transfer (in-memory first)
    double originalSenderBalance = pSenderWallet->balance;
    double originalReceiverBalance = pReceiverWallet->balance;

    pSenderWallet->balance -= amount;
    pReceiverWallet->balance += amount;
    time_t updateTime = TimeUtils::getCurrentTimestamp();
    pSenderWallet->lastUpdateTimestamp = updateTime;
    pReceiverWallet->lastUpdateTimestamp = updateTime;

    // Attempt to save. This section needs true atomicity for production.
    // For this project, we save wallets then transactions.
    if (fileHandler.saveWallets(wallets)) {
        tx.status = TransactionStatus::Successful;
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) {
            outMessage = "Points transferred successfully!";
            LOG_INFO(outMessage + " TxID: " + tx.transactionId + " Amount: " + std::to_string(amount));
            return true;
        } else {
            // Wallets saved, but transaction log failed. CRITICAL.
            outMessage = "Transfer successful, but failed to record transaction log. Please contact support.";
            LOG_ERROR("CRITICAL: Wallets updated for TxID " + tx.transactionId + " but transaction log FAILED to save.");
            // Attempt to rollback wallet changes is complex and risky here without a proper journal.
            // For now, accept data inconsistency and log heavily.
            return true; // Or false, depending on desired behavior on partial failure
        }
    } else {
        // Failed to save wallets. Rollback in-memory changes.
        pSenderWallet->balance = originalSenderBalance;
        pReceiverWallet->balance = originalReceiverBalance;
        // Don't change lastUpdateTimestamp on rollback
        outMessage = "Failed to save wallet updates. Transfer rolled back.";
        tx.status = TransactionStatus::FailedSystemError;
        transactions.push_back(tx);
        fileHandler.saveTransactions(transactions); // Log failed transaction attempt
        LOG_ERROR("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }
}

std::vector<Transaction> WalletService::getTransactionHistory(const std::string& walletId) const {
    std::vector<Transaction> history;
    for (const auto& tx : transactions) {
        if (tx.senderWalletId == walletId || tx.receiverWalletId == walletId) {
            history.push_back(tx);
        }
    }
    std::sort(history.begin(), history.end(), [](const Transaction& a, const Transaction& b) {
        return a.transactionTimestamp > b.transactionTimestamp; // Newest first
    });
    return history;
}

bool WalletService::depositPoints(const std::string& targetWalletId, double amount, 
                                  const std::string& description, const std::string& initiatedByUserId,
                                  std::string& outMessage, 
                                  const std::string& sourceWalletId) {
    if (amount <= 0) {
        outMessage = "Deposit amount must be positive.";
        LOG_WARNING("Deposit attempt failed: " + outMessage);
        return false;
    }

    Wallet* pTargetWallet = nullptr;
    for (auto& w : wallets) {
        if (w.walletId == targetWalletId) pTargetWallet = &w;
    }

    if (!pTargetWallet) {
        outMessage = "Target wallet for deposit not found.";
        LOG_WARNING("Deposit failed: " + outMessage + " Target Wallet ID: " + targetWalletId);
        return false;
    }

    // Optional: Handle source wallet logic (e.g., deducting from a master wallet)
    // For now, we assume points can be 'created' by a system deposit or admin.
    // If sourceWalletId is MASTER_WALLET_ID, you might want to check its balance
    // or just log it as the source.

    Transaction tx;
    tx.transactionId = "DEP-" + hashUtils.generateUUID().substr(0,12);
    tx.senderWalletId = sourceWalletId; // e.g., AppConfig::MASTER_WALLET_ID or "SYSTEM"
    tx.receiverWalletId = targetWalletId;
    tx.amountTransferred = amount;
    tx.transactionTimestamp = TimeUtils::getCurrentTimestamp();
    tx.description = description + " (Initiated by: " + initiatedByUserId + ")";
    tx.status = TransactionStatus::Successful;

    double originalTargetBalance = pTargetWallet->balance;
    pTargetWallet->balance += amount;
    pTargetWallet->lastUpdateTimestamp = tx.transactionTimestamp;

    if (fileHandler.saveWallets(wallets)) {
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) {
            outMessage = "Points deposited successfully.";
            LOG_INFO(outMessage + " Target Wallet: " + targetWalletId + ", Amount: " + std::to_string(amount));
            return true;
        } else {
            outMessage = "Deposit successful, but failed to record transaction log. Please contact support.";
            LOG_ERROR("CRITICAL: Wallet " + targetWalletId + " updated for deposit TxID " + tx.transactionId + " but transaction log FAILED to save.");
            return true; // Or false
        }
    } else {
        pTargetWallet->balance = originalTargetBalance; // Rollback
        outMessage = "Failed to save wallet update for deposit. Deposit rolled back.";
        LOG_ERROR("Deposit failed for target wallet " + targetWalletId + ": " + outMessage);
        // Don't save a failed transaction log here unless you want to explicitly log a system failure before any change.
        return false;
    }
}