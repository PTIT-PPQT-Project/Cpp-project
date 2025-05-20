// src/utils/Logger.cpp
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/TimeUtils.hpp" // Using our TimeUtils
#include <filesystem> // For ensureDirectoryExists in C++17

// Helper function (can be moved to a common utility if used elsewhere)
void EnsureDirectoryForFileExists(const std::string& filePath) {
    if (filePath.empty()) return;
    std::filesystem::path pathObj(filePath);
    std::filesystem::path dir = pathObj.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        try {
            std::filesystem::create_directories(dir);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Logger Error: Could not create directory " << dir.string() << ": " << e.what() << std::endl;
        }
    }
}


Logger::Logger(const std::string& filePath, LogLevel consoleLevel, LogLevel fileLevel, bool enableConsole)
    : logFilePath(filePath), currentConsoleLogLevel(consoleLevel), currentFileLogLevel(fileLevel), consoleOutputEnabled(enable) {
    if (!logFilePath.empty()) {
        EnsureDirectoryForFileExists(logFilePath);
        logFile.open(logFilePath, std::ios::out | std::ios::app); // Open in append mode
        if (!logFile.is_open()) {
            std::cerr << "Logger Error: Failed to open log file: " << logFilePath << std::endl;
        }
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

// Simple static instance for getInstance()
// This is a basic way to provide a global-like logger.
// Be mindful of initialization order if used in static contexts from other files.
Logger& Logger::getInstance(const std::string& logFilePath, LogLevel consoleLevel, LogLevel fileLevel, bool consoleLogging) {
    static Logger instance(logFilePath, consoleLevel, fileLevel, consoleLogging);
    return instance;
}


std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestampString() const {
    // Using our TimeUtils to format the timestamp
    return TimeUtils::formatTimestamp(TimeUtils::getCurrentTimestamp());
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex); // Thread safety

    std::string formattedMessage = getCurrentTimestampString() + " [" + logLevelToString(level) + "] " + message;

    if (consoleOutputEnabled && static_cast<int>(level) >= static_cast<int>(currentConsoleLogLevel)) {
        if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }

    if (logFile.is_open() && static_cast<int>(level) >= static_cast<int>(currentFileLogLevel)) {
        logFile << formattedMessage << std::endl;
        // logFile.flush(); // Optional: flush immediately, but can impact performance
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::setConsoleLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentConsoleLogLevel = level;
}

void Logger::setFileLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentFileLogLevel = level;
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex);
    consoleOutputEnabled = enable;
}