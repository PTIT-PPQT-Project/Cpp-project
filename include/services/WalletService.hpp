// include/services/WalletService.hpp
#pragma once

#include <string>
#include <vector>
#include <optional>
#include "models/User.hpp"
#include "models/Wallet.hpp"
#include "models/Transaction.hpp"
#include "services/OTPService.hpp"

class FileHandler; // Forward declaration
class HashUtils;   // Forward declaration (for generating transaction IDs)

class WalletService {
private:
    std::vector<User>& users; // Needed to get user's OTP secret
    std::vector<Wallet>& wallets;
    std::vector<Transaction>& transactions;
    FileHandler& fileHandler;
    OTPService& otpService;
    HashUtils& hashUtils; // For generating unique transaction IDs

public:
    WalletService(std::vector<User>& u_ref, std::vector<Wallet>& w_ref, 
                  std::vector<Transaction>& t_ref, FileHandler& fh_ref, 
                  OTPService& otp_ref, HashUtils& hu_ref);

    bool createWalletForUser(const std::string& userId, std::string& outMessage);

    std::optional<Wallet> getWalletByUserId(const std::string& userId) const;
    std::optional<Wallet> getWalletByWalletId(const std::string& walletId) const;

    bool transferPoints(const std::string& senderUserId, // To get OTP secret and verify ownership
                        const std::string& senderWalletId,
                        const std::string& receiverWalletId, double amount,
                        const std::string& otpCode, std::string& outMessage);

    std::vector<Transaction> getTransactionHistory(const std::string& walletId) const;
    
    bool depositPoints(const std::string& targetWalletId, double amount, 
                       const std::string& description, const std::string& initiatedByUserId, // For logging/audit, could be "SYSTEM" or adminId
                       std::string& outMessage, 
                       const std::string& sourceWalletId = AppConfig::SYSTEM_WALLET_ID_FOR_DEPOSITS);
};