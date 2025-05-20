// include/utils/Logger.hpp
#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

// LogLevel enum is defined here, and Config.h includes this file to use it.
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    // Constructor now doesn't rely on AppConfig for defaults in its declaration here.
    // Defaults can be applied in the .cpp or when called from main.
    Logger(const std::string& logFilePath,
           LogLevel consoleLogLevel,
           LogLevel fileLogLevel,
           bool enableConsoleLogging);
    ~Logger();

    // getInstance declaration: Defaults here are basic.
    // The actual configured defaults from AppConfig will be used
    // by the static instance initialization within Logger.cpp.
    static Logger& getInstance(const std::string& logFilePath = "", // Empty means Logger.cpp will use AppConfig
                               LogLevel consoleLevel = LogLevel::INFO, // Basic default
                               LogLevel fileLevel = LogLevel::DEBUG,   // Basic default
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
    std::ofstream logFileStream;
    std::string logFilePathStr;
    LogLevel currentConsoleLogLevel;
    LogLevel currentFileLogLevel;
    bool consoleOutputEnabled;
    std::mutex logMutex;

    std::string logLevelToString(LogLevel level) const;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Helper macros (không thay đổi)
#define LOG_DEBUG(message) Logger::getInstance().debug(message)
#define LOG_INFO(message) Logger::getInstance().info(message)
#define LOG_WARNING(message) Logger::getInstance().warning(message)
#define LOG_ERROR(message) Logger::getInstance().error(message)