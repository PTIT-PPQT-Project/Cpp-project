// src/utils/FileHandler.cpp
#include "utils/FileHandler.hpp"
#include "Config.h"                 // <<< THÊM ĐỂ DÙNG AppConfig
#include "utils/Logger.hpp"         // Để sử dụng LOG_INFO, LOG_ERROR
#include <filesystem>             

// Giả định ModelJsonSerialization.hpp đã được include ở đâu đó hoặc các hàm to/from_json
// được định nghĩa trong header của model.
// Nếu chưa, bạn cần include chúng ở đây:
// #include "models/ModelJsonSerialization.hpp" // Ví dụ


void FileHandler::ensureDirectoryExistsForFile(const std::string& filePath) {
    if (filePath.empty()) return;
    std::filesystem::path pathObj(filePath);
    std::filesystem::path dir = pathObj.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        try {
            if (std::filesystem::create_directories(dir)) {
                LOG_INFO("FileHandler: Created directory: " + dir.string());
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("FileHandler: Could not create directory " + dir.string() + ": " + e.what());
        }
    }
}

FileHandler::FileHandler() {
    std::string baseDir = AppConfig::DATA_DIRECTORY;
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
        baseDir += "/";
    }
    // Đảm bảo thư mục data/ tồn tại
    if (!baseDir.empty()) { // Check if baseDir is not just "/" or empty
         ensureDirectoryExistsForFile(baseDir + "dummy_check.txt"); // To create baseDir
    }


    usersFilePath = baseDir + AppConfig::USERS_FILENAME;
    walletsFilePath = baseDir + AppConfig::WALLETS_FILENAME;
    transactionsFilePath = baseDir + AppConfig::TRANSACTIONS_FILENAME;

    LOG_INFO("FileHandler initialized. Users: " + usersFilePath + ", Wallets: " + walletsFilePath + ", Transactions: " + transactionsFilePath);
}

// --- User Data ---
bool FileHandler::loadUsers(std::vector<User>& users) {
    users.clear();
    std::ifstream file(usersFilePath);
    if (!file.is_open()) {
        LOG_INFO("Users file (" + usersFilePath + ") not found. Starting with empty list.");
        return true; 
    }
    try {
        json j;
        file >> j; 
        if (j.is_array()) {
            users = j.get<std::vector<User>>();
            LOG_INFO("Loaded " + std::to_string(users.size()) + " users from " + usersFilePath);
        } else if (j.is_null() || j.empty()) { 
             LOG_INFO("Users file (" + usersFilePath + ") is null or empty.");
        } else {
            LOG_ERROR("Users file (" + usersFilePath + ") does not contain a valid JSON array. Content: " + j.dump());
            file.close();
            return false;
        }
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error in users file ("+ usersFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error in users file ("+ usersFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error loading users from ("+ usersFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveUsers(const std::vector<User>& users) {
    ensureDirectoryExistsForFile(usersFilePath);
    std::ofstream file(usersFilePath);
    if (!file.is_open()) {
        LOG_ERROR("Could not open users file for writing: " + usersFilePath);
        return false;
    }
    try {
        json j = users; 
        file << std::setw(4) << j << std::endl; // Pretty print
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error saving users: " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error saving users to ("+ usersFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

// --- Wallet Data ---
bool FileHandler::loadWallets(std::vector<Wallet>& wallets) {
    wallets.clear();
    std::ifstream file(walletsFilePath);
    if (!file.is_open()) {
        LOG_INFO("Wallets file (" + walletsFilePath + ") not found. Starting empty.");
        return true;
    }
    try {
        json j;
        file >> j;
        if (j.is_array()) {
            wallets = j.get<std::vector<Wallet>>();
            LOG_INFO("Loaded " + std::to_string(wallets.size()) + " wallets from " + walletsFilePath);
        } else if (j.is_null() || j.empty()) {
             LOG_INFO("Wallets file (" + walletsFilePath + ") is null or empty.");
        } else {
            LOG_ERROR("Wallets file (" + walletsFilePath + ") is not a valid JSON array. Content: " + j.dump());
            file.close();
            return false;
        }
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error in wallets file ("+ walletsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error in wallets file ("+ walletsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error loading wallets ("+ walletsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveWallets(const std::vector<Wallet>& wallets) {
    ensureDirectoryExistsForFile(walletsFilePath);
    std::ofstream file(walletsFilePath);
    if (!file.is_open()) {
        LOG_ERROR("Could not open wallets file for writing: " + walletsFilePath);
        return false;
    }
    try {
        json j = wallets;
        file << std::setw(4) << j << std::endl;
        LOG_INFO("Saved " + std::to_string(wallets.size()) + " wallets to " + walletsFilePath);
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error saving wallets: " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error saving wallets ("+ walletsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

// --- Transaction Data ---
bool FileHandler::loadTransactions(std::vector<Transaction>& transactions) {
    transactions.clear();
    std::ifstream file(transactionsFilePath);
    if (!file.is_open()) {
        LOG_INFO("Transactions file (" + transactionsFilePath + ") not found. Starting empty.");
        return true;
    }
    try {
        json j;
        file >> j;
        if (j.is_array()) {
            transactions = j.get<std::vector<Transaction>>();
            LOG_INFO("Loaded " + std::to_string(transactions.size()) + " transactions from " + transactionsFilePath);
        } else if (j.is_null() || j.empty()) {
            LOG_INFO("Transactions file (" + transactionsFilePath + ") is null or empty.");
        } else {
            LOG_ERROR("Transactions file (" + transactionsFilePath + ") is not a valid JSON array. Content: " + j.dump());
            file.close();
            return false;
        }
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error in transactions file ("+ transactionsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error in transactions file ("+ transactionsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error loading transactions ("+ transactionsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveTransactions(const std::vector<Transaction>& transactions) {
    ensureDirectoryExistsForFile(transactionsFilePath);
    std::ofstream file(transactionsFilePath);
    if (!file.is_open()) {
        LOG_ERROR("Could not open transactions file for writing: " + transactionsFilePath);
        return false;
    }
    try {
        json j = transactions;
        file << std::setw(4) << j << std::endl;
        LOG_INFO("Saved " + std::to_string(transactions.size()) + " transactions to " + transactionsFilePath);
    } catch (const json::type_error& e) {
        LOG_ERROR("JSON type error saving transactions: " + std::string(e.what()));
        file.close();
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Generic error saving transactions ("+ transactionsFilePath +"): " + std::string(e.what()));
        file.close();
        return false;
    }
    file.close();
    return true;
}