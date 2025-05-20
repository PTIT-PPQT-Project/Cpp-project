// src/utils/FileHandler.cpp
#include "../../include/utils/FileHandler.hpp"
#include <filesystem> // For std::filesystem::create_directories (C++17)
                      // If not C++17, you might need OS-specific directory creation or a library.


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
    std::string baseDir = dataDir;
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
        baseDir += "/";
    }

    usersFilePath = baseDir + "users.json";
    walletsFilePath = baseDir + "wallets.json";
    transactionsFilePath = baseDir + "transactions.json";

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
        // If file doesn't exist, it's not an error, just no users loaded.
        // Could create an empty file here if desired.
        std::cerr << "[FileHandler] INFO: Users file (" << usersFilePath << ") not found. Starting with empty user list." << std::endl;
        return true; // Not an error if file simply doesn't exist yet
    }

    try {
        json j;
        file >> j;
        // Assuming the top level is an array of users
        if (j.is_array()) {
            users = j.get<std::vector<User>>(); // Uses from_json for User
        } else if (j.is_null()) { // Handle empty file or file with just 'null'
             std::cerr << "[FileHandler] INFO: Users file (" << usersFilePath << ") is null or empty. Starting with empty user list." << std::endl;
        }
         else {
            std::cerr << "[FileHandler] ERROR: Users file (" << usersFilePath << ") does not contain a valid JSON array." << std::endl;
            return false;
        }
    } catch (json::parse_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON parse error in users file: " << e.what() << std::endl;
        return false;
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error in users file (likely malformed data): " << e.what() << std::endl;
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error loading users: " << e.what() << std::endl;
        return false;
    }
    file.close();
    return true;
}

bool FileHandler::saveUsers(const std::vector<User>& users) {
    ensureDirectoryExists(usersFilePath);
    std::ofstream file(usersFilePath);
    if (!file.is_open()) {
        std::cerr << "[FileHandler] ERROR: Could not open users file for writing: " << usersFilePath << std::endl;
        return false;
    }
    try {
        json j = users; // Uses to_json for User (and std::vector<User>)
        file << j.dump(4); // Pretty print with 4 spaces indent
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error saving users (data conversion failed): " << e.what() << std::endl;
        file.close(); // Close file on error
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error saving users: " << e.what() << std::endl;
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
    std::ofstream file(transactionsFilePath);
    if (!file.is_open()) {
        std::cerr << "[FileHandler] ERROR: Could not open transactions file for writing: " << transactionsFilePath << std::endl;
        return false;
    }
    try {
        json j = transactions;
        file << j.dump(4);
    } catch (json::type_error& e) {
        std::cerr << "[FileHandler] ERROR: JSON type error saving transactions: " << e.what() << std::endl;
        file.close();
        return false;
    } catch (std::exception& e) {
        std::cerr << "[FileHandler] ERROR: Generic error saving transactions: " << e.what() << std::endl;
        file.close();
        return false;
    }
    file.close();
    return true;
}