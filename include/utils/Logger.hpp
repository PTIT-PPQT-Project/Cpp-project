// include/utils/Logger.hpp
#pragma once

#include <string>
#include <fstream>
#include <mutex>     // For thread-safe logging (if needed in the future)
#include <iostream>  // For std::cout, std::cerr

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    // Constructor: allows specifying a log file and minimum log level for file/console
    Logger(const std::string& logFilePath = "",
           LogLevel consoleLogLevel = LogLevel::INFO,
           LogLevel fileLogLevel = LogLevel::DEBUG,
           bool enableConsoleLogging = true);
    ~Logger();

    // Static method to get a shared instance (simple singleton-like access)
    // For more robust singleton, consider Meyer's Singleton or other patterns.
    // This simple static instance is often enough for basic needs.
    // If you prefer DI, then don't use this and pass Logger instances around.
    static Logger& getInstance(const std::string& logFilePath = "app.log", 
                               LogLevel consoleLevel = LogLevel::INFO, 
                               LogLevel fileLevel = LogLevel::DEBUG,
                               bool consoleLogging = true);


    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    void setConsoleLogLevel(LogLevel level);
    void setFileLogLevel(LogLevel level);
    void enableConsoleOutput(bool enable);

private:
    std::ofstream logFile;
    std::string logFilePath;
    LogLevel currentConsoleLogLevel;
    LogLevel currentFileLogLevel;
    bool consoleOutputEnabled;
    std::mutex logMutex; // To make logging thread-safe

    std::string logLevelToString(LogLevel level) const;
    std::string getCurrentTimestampString() const;

    // Delete copy constructor and assignment operator for singleton-like behavior via getInstance
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Helper macro for easy logging (optional)
#define LOG_DEBUG(message) Logger::getInstance().debug(message)
#define LOG_INFO(message) Logger::getInstance().info(message)
#define LOG_WARNING(message) Logger::getInstance().warning(message)
#define LOG_ERROR(message) Logger::getInstance().error(message)