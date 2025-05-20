// include/utils/FileHandler.hpp
#pragma once

#include <string>
#include <vector>
#include <fstream> 
#include <iostream> 

#include "models/User.hpp"
#include "models/Wallet.hpp"
#include "models/Transaction.hpp"
#include "nlohmann/json.hpp" 

using json = nlohmann::json;

class FileHandler {
private:
    std::string usersFilePath;
    std::string walletsFilePath;
    std::string transactionsFilePath;

    void ensureDirectoryExistsForFile(const std::string& filePath);

public:
    // Constructor sẽ sử dụng AppConfig cho đường dẫn mặc định
    FileHandler(); 

    bool loadUsers(std::vector<User>& users);
    bool saveUsers(const std::vector<User>& users);

    bool loadWallets(std::vector<Wallet>& wallets);
    bool saveWallets(const std::vector<Wallet>& wallets);

    bool loadTransactions(std::vector<Transaction>& transactions);
    bool saveTransactions(const std::vector<Transaction>& transactions);
};