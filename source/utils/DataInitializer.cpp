#include "../../include/utils/DataInitializer.hpp"
#include "../../include/utils/Logger.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

bool DataInitializer::createEmptyJsonFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << "[]";
        file.close();
        LOG_INFO("Created empty JSON file: " + filePath);
        return true;
    }
    LOG_ERROR("Failed to create empty JSON file: " + filePath);
    return false;
}

bool DataInitializer::createDataDirectory(const std::string& dataDir) {
    try {
        if (!std::filesystem::exists(dataDir)) {
            std::filesystem::create_directories(dataDir);
            LOG_INFO("Created data directory: " + dataDir);
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create data directory: " + std::string(e.what()));
        return false;
    }
}

bool DataInitializer::initializeJsonFiles(const std::string& dataDir) {
    std::string usersFile = dataDir + "users.json";
    std::string walletsFile = dataDir + "wallets.json";
    std::string transactionsFile = dataDir + "transactions.json";

    bool success = true;
    success &= createEmptyJsonFile(usersFile);
    success &= createEmptyJsonFile(walletsFile);
    success &= createEmptyJsonFile(transactionsFile);

    return success;
}

bool DataInitializer::initializeDataFiles(const std::string& dataDir) {
    // Ensure the data directory exists
    if (!createDataDirectory(dataDir)) {
        return false;
    }

    // Initialize all JSON files
    return initializeJsonFiles(dataDir);
} 