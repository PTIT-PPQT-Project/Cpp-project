#include "services/WalletService.hpp"
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeUtils.hpp"
#include "utils/InputValidator.hpp"
#include "Config.h"
#include <algorithm>
#include <sstream>

WalletService::WalletService(std::vector<User>& u_ref, std::vector<Wallet>& w_ref,
                            std::vector<Transaction>& t_ref, FileHandler& fh_ref,
                            OTPService& otp_ref, HashUtils& hu_ref)
    : users(u_ref), wallets(w_ref), transactions(t_ref),
      fileHandler(fh_ref), otpService(otp_ref), hashUtils(hu_ref) {}

namespace {
    // Helper function to find user by ID
    auto findUserById(const std::vector<User>& users, const std::string& userId) {
        return std::find_if(users.cbegin(), users.cend(),
            [&userId](const User& u) { return u.userId == userId; });
    }

    // Helper function to find wallet by ID (non-const for modification)
    auto findWalletById(std::vector<Wallet>& wallets, const std::string& walletId) {
        return std::find_if(wallets.begin(), wallets.end(),
            [&walletId](const Wallet& w) { return w.walletId == walletId; });
    }

    // Helper function to find wallet by ID (const version)
    auto findWalletByIdConst(const std::vector<Wallet>& wallets, const std::string& walletId) {
        return std::find_if(wallets.cbegin(), wallets.cend(),
            [&walletId](const Wallet& w) { return w.walletId == walletId; });
    }

    // Helper function to generate transaction ID
    std::string generateTransactionId(HashUtils& hashUtils, const std::string& prefix) {
        return prefix + hashUtils.generateUUID().substr(0, 12);
    }

    // Helper function to sanitize input (from UserService refactor)
    std::string sanitizeInput(const std::string& input) {
        std::string sanitized = input;
        sanitized.erase(0, sanitized.find_first_not_of(" \t\n\r"));
        sanitized.erase(sanitized.find_last_not_of(" \t\n\r") + 1);
        return sanitized;
    }
}

bool WalletService::createWalletForUser(const std::string& userId, std::string& outMessage) {
    if (userId.empty()) {
        outMessage = "Invalid user ID.";
        LOG_WARNING("Wallet creation failed: empty user ID");
        return false;
    }

    auto user_it = findUserById(users, userId);
    if (user_it == users.cend()) {
        outMessage = "User not found.";
        LOG_WARNING("Wallet creation failed: User ID " + userId + " not found");
        return false;
    }

    auto wallet_exists = findWalletByIdConst(wallets, userId);
    if (wallet_exists != wallets.cend()) {
        outMessage = "Wallet already exists for user " + user_it->username + ".";
        LOG_INFO("Wallet creation skipped: Wallet exists for user " + user_it->username);
        return true;
    }

    Wallet newWallet;
    newWallet.walletId = generateTransactionId(hashUtils, "WLT-");
    newWallet.userId = userId;
    newWallet.balance = AppConfig::DEFAULT_INITIAL_WALLET_BALANCE;
    newWallet.creationTimestamp = TimeUtils::getCurrentTimestamp();
    newWallet.lastUpdateTimestamp = newWallet.creationTimestamp;

    wallets.push_back(newWallet);
    if (fileHandler.saveWallets(wallets)) {
        outMessage = "Wallet created successfully for user " + user_it->username + ". Wallet ID: " + newWallet.walletId;
        LOG_INFO(outMessage);
        return true;
    }

    wallets.pop_back();
    outMessage = "Failed to save new wallet.";
    LOG_ERROR(outMessage + " User ID: " + userId);
    return false;
}

std::optional<Wallet> WalletService::getWalletByUserId(const std::string& userId) const {
    if (userId.empty()) {
        LOG_WARNING("Wallet lookup failed: empty user ID");
        return std::nullopt;
    }

    auto it = std::find_if(wallets.cbegin(), wallets.cend(),
        [&userId](const Wallet& w) { return w.userId == userId; });
    if (it != wallets.cend()) {
        return *it;
    }
    LOG_DEBUG("Wallet not found for User ID: " + userId);
    return std::nullopt;
}

std::optional<Wallet> WalletService::getWalletByWalletId(const std::string& walletId) const {
    if (walletId.empty()) {
        LOG_WARNING("Wallet lookup failed: empty wallet ID");
        return std::nullopt;
    }

    auto it = findWalletByIdConst(wallets, walletId);
    if (it != wallets.cend()) {
        return *it;
    }
    LOG_DEBUG("Wallet not found for Wallet ID: " + walletId);
    return std::nullopt;
}

bool WalletService::transferPoints(const std::string& senderUserId, const std::string& senderWalletId,
                                  const std::string& receiverWalletId, double amount,
                                  const std::string& otpCode, std::string& outMessage) {
    if (!InputValidator::isValidPositiveAmount(amount)) {
        outMessage = "Transfer amount must be positive.";
        LOG_WARNING("Transfer failed: Invalid amount " + std::to_string(amount));
        return false;
    }

    if (senderWalletId == receiverWalletId) {
        outMessage = "Cannot transfer to the same wallet.";
        LOG_WARNING("Transfer failed: Identical wallet IDs " + senderWalletId);
        return false;
    }

    if (senderUserId.empty() || senderWalletId.empty() || receiverWalletId.empty()) {
        outMessage = "Invalid user or wallet ID.";
        LOG_WARNING("Transfer failed: Empty user or wallet ID");
        return false;
    }

    auto senderUserIt = findUserById(users, senderUserId);
    if (senderUserIt == users.cend()) {
        outMessage = "Sender user not found.";
        LOG_ERROR("Transfer failed: User ID " + senderUserId + " not found");
        return false;
    }

    if (!senderUserIt->otpSecretKey.empty() && !otpService.verifyOtp(senderUserIt->otpSecretKey, otpCode)) {
        outMessage = otpCode.empty() ? "OTP required for transfer." : "Invalid OTP code.";
        LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }

    auto senderWalletIt = findWalletById(wallets, senderWalletId);
    if (senderWalletIt == wallets.end()) {
        outMessage = "Sender wallet not found.";
        LOG_WARNING("Transfer failed: Wallet ID " + senderWalletId + " not found");
        return false;
    }

    if (senderWalletIt->userId != senderUserId) {
        outMessage = "Sender wallet does not belong to the authenticated user.";
        LOG_ERROR("Transfer security alert: User '" + senderUserIt->username + "' attempted to use wallet " + senderWalletId);
        return false;
    }

    auto receiverWalletIt = findWalletById(wallets, receiverWalletId);
    if (receiverWalletIt == wallets.end()) {
        outMessage = "Receiver wallet not found.";
        LOG_WARNING("Transfer failed: Wallet ID " + receiverWalletId + " not found");
        return false;
    }

    if (senderWalletIt->balance < amount) {
        outMessage = "Insufficient balance. Available: " + std::to_string(senderWalletIt->balance) +
                     ", Required: " + std::to_string(amount);
        Transaction tx;
        tx.transactionId = generateTransactionId(hashUtils, "TXN-");
        tx.sourceWalletId = senderWalletId;
        tx.targetWalletId = receiverWalletId;
        tx.amount = amount;
        tx.timestamp = TimeUtils::getCurrentTimestamp();
        tx.description = "Failed transfer from " + senderUserIt->username + " (Wallet: " + senderWalletId +
                         ") to Wallet: " + receiverWalletId + " (Insufficient balance)";
        tx.status = TransactionStatus::Failed;
        transactions.push_back(tx);
        fileHandler.saveTransactions(transactions);
        LOG_WARNING("Transfer failed for user '" + senderUserIt->username + "': " + outMessage);
        return false;
    }

    // Store original state for rollback
    Wallet originalSender = *senderWalletIt;
    Wallet originalReceiver = *receiverWalletIt;

    // Perform in-memory transfer
    time_t updateTime = TimeUtils::getCurrentTimestamp();
    senderWalletIt->balance -= amount;
    receiverWalletIt->balance += amount;
    senderWalletIt->lastUpdateTimestamp = updateTime;
    receiverWalletIt->lastUpdateTimestamp = updateTime;

    Transaction tx;
    tx.transactionId = generateTransactionId(hashUtils, "TXN-");
    tx.sourceWalletId = senderWalletId;
    tx.targetWalletId = receiverWalletId;
    tx.amount = amount;
    tx.timestamp = updateTime;
    tx.description = "Transfer from " + senderUserIt->username + " (Wallet: " + senderWalletId +
                     ") to Wallet: " + receiverWalletId;
    tx.status = TransactionStatus::Completed;

    // Save wallets and transaction atomically
    if (fileHandler.saveWallets(wallets)) {
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) {
            outMessage = "Points transferred successfully. TxID: " + tx.transactionId;
            LOG_INFO(outMessage + ", Amount: " + std::to_string(amount));
            return true;
        }
        // Rollback wallet changes if transaction save fails
        *senderWalletIt = originalSender;
        *receiverWalletIt = originalReceiver;
        fileHandler.saveWallets(wallets); // Persist rollback
        outMessage = "Transfer failed: Unable to record transaction.";
        LOG_ERROR(outMessage + " TxID: " + tx.transactionId + ". Wallets rolled back.");
        return false;
    }

    // Rollback in-memory changes
    *senderWalletIt = originalSender;
    *receiverWalletIt = originalReceiver;
    tx.status = TransactionStatus::Failed;
    transactions.push_back(tx);
    fileHandler.saveTransactions(transactions);
    outMessage = "Transfer failed: Unable to save wallet updates.";
    LOG_ERROR(outMessage + " TxID: " + tx.transactionId);
    return false;
}

std::vector<Transaction> WalletService::getTransactionHistory(const std::string& walletId) const {
    if (walletId.empty()) {
        LOG_WARNING("Transaction history request failed: empty wallet ID");
        return {};
    }

    std::vector<Transaction> history;
    std::copy_if(transactions.begin(), transactions.end(), std::back_inserter(history),
        [&walletId](const Transaction& tx) {
            return tx.sourceWalletId == walletId || tx.targetWalletId == walletId;
        });

    std::sort(history.begin(), history.end(),
        [](const Transaction& a, const Transaction& b) { return a.timestamp > b.timestamp; });

    LOG_DEBUG("Retrieved " + std::to_string(history.size()) + " transactions for Wallet ID: " + walletId);
    return history;
}

bool WalletService::depositPoints(const std::string& targetWalletId, double amount,
                                 const std::string& description, const std::string& initiatedByUserId,
                                 std::string& outMessage, const std::string& sourceWalletId) {
    if (!InputValidator::isValidPositiveAmount(amount)) {
        outMessage = "Deposit amount must be positive.";
        LOG_WARNING("Deposit failed: Invalid amount " + std::to_string(amount));
        return false;
    }

    if (targetWalletId.empty() || initiatedByUserId.empty()) {
        outMessage = "Invalid wallet or user ID.";
        LOG_WARNING("Deposit failed: Empty wallet or user ID");
        return false;
    }

    auto targetWalletIt = findWalletById(wallets, targetWalletId);
    if (targetWalletIt == wallets.end()) {
        outMessage = "Target wallet not found.";
        LOG_WARNING("Deposit failed: Wallet ID " + targetWalletId + " not found");
        return false;
    }

    Wallet originalWallet = *targetWalletIt;
    std::string sanitizedDescription = sanitizeInput(description);

    // Update wallet
    time_t updateTime = TimeUtils::getCurrentTimestamp();
    targetWalletIt->balance += amount;
    targetWalletIt->lastUpdateTimestamp = updateTime;

    Transaction tx;
    tx.transactionId = generateTransactionId(hashUtils, "DEP-");
    tx.sourceWalletId = sourceWalletId;
    tx.targetWalletId = targetWalletId;
    tx.amount = amount;
    tx.timestamp = updateTime;
    tx.description = sanitizedDescription.empty() ? "Deposit (Initiated by: " + initiatedByUserId + ")" :
                     sanitizedDescription + " (Initiated by: " + initiatedByUserId + ")";
    tx.status = TransactionStatus::Completed;

    if (fileHandler.saveWallets(wallets)) {
        transactions.push_back(tx);
        if (fileHandler.saveTransactions(transactions)) {
            outMessage = "Deposit successful. New balance: " + std::to_string(targetWalletIt->balance);
            LOG_INFO(outMessage + " for wallet " + targetWalletId + ", Amount: " + std::to_string(amount));
            return true;
        }
        // Rollback wallet changes
        *targetWalletIt = originalWallet;
        fileHandler.saveWallets(wallets);
        outMessage = "Deposit failed: Unable to record transaction.";
        LOG_ERROR(outMessage + " for wallet " + targetWalletId);
        return false;
    }

    *targetWalletIt = originalWallet;
    tx.status = TransactionStatus::Failed;
    transactions.push_back(tx);
    fileHandler.saveTransactions(transactions);
    outMessage = "Deposit failed: Unable to save wallet updates.";
    LOG_ERROR(outMessage + " for wallet " + targetWalletId);
    return false;
}