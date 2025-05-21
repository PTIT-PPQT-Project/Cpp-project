#pragma once

#include <string>
#include <vector>
#include "FileHandler.hpp"

class DataInitializer {
public:
    static bool initializeDataFiles(FileHandler& fileHandler);
    static bool createDataDirectory();

private:
    static bool initializeUsersFile(FileHandler& fileHandler);
    static bool initializeWalletsFile(FileHandler& fileHandler);
    static bool initializeTransactionsFile(FileHandler& fileHandler);
}; 