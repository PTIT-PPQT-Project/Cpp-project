#include "utils/DataInitializer.hpp"
#include "utils/Logger.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

bool DataInitializer::createDataDirectory() {
    try {
        if (!fs::exists("data")) {
            fs::create_directory("data");
            LOG_INFO("Created data directory");
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create data directory: " + std::string(e.what()));
        return false;
    }
}

bool DataInitializer::initializeUsersFile(FileHandler& fileHandler) {
    std::vector<User> users;
    if (!fileHandler.loadUsers(users)) {
        // Create empty users file
        if (!fileHandler.saveUsers(users)) {
            LOG_ERROR("Failed to initialize users.json");
            return false;
        }
        LOG_INFO("Initialized empty users.json");
    }
    return true;
}

bool DataInitializer::initializeWalletsFile(FileHandler& fileHandler) {
    std::vector<Wallet> wallets;
    if (!fileHandler.loadWallets(wallets)) {
        // Create empty wallets file
        if (!fileHandler.saveWallets(wallets)) {
            LOG_ERROR("Failed to initialize wallets.json");
            return false;
        }
        LOG_INFO("Initialized empty wallets.json");
    }
    return true;
}

bool DataInitializer::initializeTransactionsFile(FileHandler& fileHandler) {
    std::vector<Transaction> transactions;
    if (!fileHandler.loadTransactions(transactions)) {
        // Create empty transactions file
        if (!fileHandler.saveTransactions(transactions)) {
            LOG_ERROR("Failed to initialize transactions.json");
            return false;
        }
        LOG_INFO("Initialized empty transactions.json");
    }
    return true;
}

bool DataInitializer::initializeDataFiles(FileHandler& fileHandler) {
    if (!createDataDirectory()) {
        return false;
    }

    return initializeUsersFile(fileHandler) &&
           initializeWalletsFile(fileHandler) &&
           initializeTransactionsFile(fileHandler);
} 