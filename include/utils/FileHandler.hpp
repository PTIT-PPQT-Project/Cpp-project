// include/utils/FileHandler.hpp
#pragma once

#include <string>
#include <vector>
#include <fstream>  // For std::ifstream, std::ofstream
#include <iostream> // For error messages

#include "../models/User.hpp"
#include "../models/Wallet.hpp"
#include "../models/Transaction.hpp"
#include "nlohmann/json.hpp" // Assuming this is in your include path or vendored

// Use a shorter alias for nlohmann::json
using json = nlohmann::json;

class FileHandler {
private:
    std::string usersFilePath;
    std::string walletsFilePath;
    std::string transactionsFilePath;

    // Helper to create directories if they don't exist
    void ensureDirectoryExists(const std::string& filePath);

public:
    FileHandler(const std::string& dataDir = "data/"); // Constructor with default data directory

    // User data
    bool loadUsers(std::vector<User>& users);
    bool saveUsers(const std::vector<User>& users);

    // Wallet data
    bool loadWallets(std::vector<Wallet>& wallets);
    bool saveWallets(const std::vector<Wallet>& wallets);

    // Transaction data
    bool loadTransactions(std::vector<Transaction>& transactions);
    bool saveTransactions(const std::vector<Transaction>& transactions);
};