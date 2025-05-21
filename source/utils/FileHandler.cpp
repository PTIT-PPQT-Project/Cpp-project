// src/utils/FileHandler.cpp
#include "../../include/utils/FileHandler.hpp"
#include "../../include/utils/Logger.hpp"
#include <filesystem> // For std::filesystem::create_directories (C++17)
                      // If not C++17, you might need OS-specific directory creation or a library.
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// IMPORTANT: Ensure the to_json and from_json functions for User, Wallet, Transaction,
// and their enums are defined and accessible here.
// Typically, you would include a header that defines them, or define them in the model's .cpp files
// and include nlohmann/json.hpp there.
// For this example, I've provided them above as free functions. If you put them in a separate header,
// e.g., "ModelJsonSerialization.hpp", include it here:
// #include "../../include/models/ModelJsonSerialization.hpp" // Example

// Helper function to ensure directory for a file path exists
void FileHandler::ensureDirectoryExists(const std::string& filePath) {
    std::filesystem::path pathObj(filePath);
    std::filesystem::path dir = pathObj.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}


FileHandler::FileHandler(const std::string& dataDir) {
    // Get the project root directory (two levels up from build)
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path projectRoot = currentPath.parent_path().parent_path();
    
    // Ensure we're in the Cpp-project directory
    if (projectRoot.filename() != "Cpp-project") {
        // Try to find Cpp-project in the path
        std::filesystem::path tempPath = currentPath;
        while (!tempPath.empty() && tempPath.filename() != "Cpp-project") {
            tempPath = tempPath.parent_path();
        }
        if (!tempPath.empty()) {
            projectRoot = tempPath;
        }
    }
    
    std::string baseDir = (projectRoot / "data").string();
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
        baseDir += "/";
    }

    usersFilePath = baseDir + "users.json";
    walletsFilePath = baseDir + "wallets.json";
    transactionsFilePath = baseDir + "transactions.json";

    // Get absolute paths for logging
    std::filesystem::path absUsersPath = std::filesystem::absolute(usersFilePath);
    std::filesystem::path absWalletsPath = std::filesystem::absolute(walletsFilePath);
    std::filesystem::path absTransactionsPath = std::filesystem::absolute(transactionsFilePath);

    LOG_INFO("FileHandler initialized with absolute paths:");
    LOG_INFO("Current path: " + currentPath.string());
    LOG_INFO("Project root: " + projectRoot.string());
    LOG_INFO("Users file: " + absUsersPath.string());
    LOG_INFO("Wallets file: " + absWalletsPath.string());
    LOG_INFO("Transactions file: " + absTransactionsPath.string());

    // Ensure directories exist when FileHandler is created
    ensureDirectoryExists(usersFilePath);
    ensureDirectoryExists(walletsFilePath);
    ensureDirectoryExists(transactionsFilePath);
}

// --- User Data ---
bool FileHandler::loadUsers(std::vector<User>& users) {
    users.clear();
    std::ifstream file(usersFilePath);
    if (!file.is_open()) {
        // If file doesn't exist, create it with an empty array
        ensureDirectoryExists(usersFilePath);
        std::ofstream newFile(usersFilePath);
        if (newFile.is_open()) {
            newFile << "[]";
            newFile.close();
        }
        return true; // Not an error if file simply doesn't exist yet
    }

    try {
        json j;
        file >> j; 
        
        // Assuming the top level is an array of users
        if (j.is_array()) {
            users = j.get<std::vector<User>>(); // Uses from_json for User
        } else if (j.is_null()) { // Handle empty file or file with just 'null'
            // Empty file is not an error
        } else {
            LOG_ERROR("Users file does not contain a valid JSON array.");
            return false;
        }
    } catch (json::parse_error& e) {
        LOG_ERROR("JSON parse error in users file: " + std::string(e.what()));
        return false;
    } catch (json::type_error& e) {
        LOG_ERROR("JSON type error in users file: " + std::string(e.what()));
        return false;
    } catch (std::exception& e) {
        LOG_ERROR("Error loading users: " + std::string(e.what()));
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveUsers(const std::vector<User>& users) {
    try {
        ensureDirectoryExists(usersFilePath);
        json j = users; // Serialize users to JSON
        
        std::ofstream file(usersFilePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open users file for writing");
            return false;
        }
        
        file << j.dump(4); // Pretty print JSON
        file.flush(); // Ensure data is written to disk
        file.close();
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error saving users: " + std::string(e.what()));
        return false;
    }
}

// --- Wallet Data ---
bool FileHandler::loadWallets(std::vector<Wallet>& wallets) {
    wallets.clear();
    std::ifstream file(walletsFilePath);
    if (!file.is_open()) {
        std::cerr << "[FileHandler] INFO: Wallets file (" << walletsFilePath << ") not found. Starting with empty wallet list." << std::endl;
        return true;
    }
    try {
        json j;
        file >> j;
        if (j.is_array()) {
            wallets = j.get<std::vector<Wallet>>();
        } else if (j.is_null()) {
             std::cerr << "[FileHandler] INFO: Wallets file (" << walletsFilePath << ") is null or empty." << std::endl;
        } else {
            std::cerr << "[FileHandler] ERROR: Wallets file (" << walletsFilePath << ") does not contain a valid JSON array." << std::endl;
            return false;
        }
    } catch (json::parse_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON parse error in wallets file: " << e.what() << std::endl;
        return false;
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error in wallets file: " << e.what() << std::endl;
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error loading wallets: " << e.what() << std::endl;
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveWallets(const std::vector<Wallet>& wallets) {
    ensureDirectoryExists(walletsFilePath);
    std::ofstream file(walletsFilePath);
    if (!file.is_open()) {
        std::cerr << "[FileHandler] ERROR: Could not open wallets file for writing: " << walletsFilePath << std::endl;
        return false;
    }
    try {
        json j = wallets;
        file << j.dump(4);
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error saving wallets: " << e.what() << std::endl;
        file.close();
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error saving wallets: " << e.what() << std::endl;
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
        std::cerr << "[FileHandler] INFO: Transactions file (" << transactionsFilePath << ") not found. Starting with empty transaction list." << std::endl;
        return true;
    }
    try {
        json j;
        file >> j;
        if (j.is_array()) {
            transactions = j.get<std::vector<Transaction>>();
        } else if (j.is_null()) {
            std::cerr << "[FileHandler] INFO: Transactions file (" << transactionsFilePath << ") is null or empty." << std::endl;
        } else {
            std::cerr << "[FileHandler] ERROR: Transactions file (" << transactionsFilePath << ") does not contain a valid JSON array." << std::endl;
            return false;
        }
    } catch (json::parse_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON parse error in transactions file: " << e.what() << std::endl;
        return false;
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error in transactions file: " << e.what() << std::endl;
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error loading transactions: " << e.what() << std::endl;
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveTransactions(const std::vector<Transaction>& transactions) {
    ensureDirectoryExists(transactionsFilePath);
    std::ofstream file(transactionsFilePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("Could not open transactions file for writing: " + transactionsFilePath);
        return false;
    }
    try {
        json j = transactions;
        file << j.dump(4);
        file.flush(); // Ensure data is written to disk
        file.close();
        LOG_INFO("Successfully saved " + std::to_string(transactions.size()) + " transactions to file");
        return true;
    } catch (json::type_error& e) {
        LOG_ERROR("JSON type error saving transactions: " + std::string(e.what()));
        file.close();
        return false;
    } catch (std::exception& e) {
        LOG_ERROR("Generic error saving transactions: " + std::string(e.what()));
        file.close();
        return false;
    }
}