#pragma once

#include <string>

class DataInitializer {
public:
    static bool initializeDataFiles(const std::string& dataDir = "data/");
    static bool createDataDirectory(const std::string& dataDir = "data/");
    static bool initializeJsonFiles(const std::string& dataDir = "data/");

private:
    static bool createEmptyJsonFile(const std::string& filePath);
}; 