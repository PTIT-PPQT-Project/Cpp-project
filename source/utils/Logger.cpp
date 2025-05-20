// src/utils/Logger.cpp
#include "utils/Logger.hpp"
#include "utils/TimeUtils.hpp"
#include "Config.h"           // <<< THÊM ĐỂ DÙNG AppConfig
#include <filesystem>
#include <iostream>

namespace {
    void EnsureDirectoryForFileExistsHelperInternal(const std::string& filePath) {
        if (filePath.empty()) return;
        std::filesystem::path pathObj(filePath);
        std::filesystem::path dir = pathObj.parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            try {
                std::filesystem::create_directories(dir);
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Logger Internal Error: Could not create directory " << dir.string() << ": " << e.what() << std::endl;
            }
        }
    }
}

// Constructor: Sử dụng các giá trị đã được truyền vào
Logger::Logger(const std::string& filePath, LogLevel consoleLevel, LogLevel fileLevel, bool enableConsole)
    : logFilePathStr(filePath), currentConsoleLogLevel(consoleLevel), currentFileLogLevel(fileLevel), consoleOutputEnabled(enableConsole) {
    if (!logFilePathStr.empty()) {
        EnsureDirectoryForFileExistsHelperInternal(logFilePathStr);
        logFileStream.open(logFilePathStr, std::ios::out | std::ios::app);
        if (!logFileStream.is_open()) {
            std::cerr << "Logger Error: Failed to open log file: " << logFilePathStr << std::endl;
        } else {
            logFileStream << TimeUtils::formatTimestamp(TimeUtils::getCurrentTimestamp())
                          << " [INFO] Logger initialized. Logging to file: " << logFilePathStr << std::endl;
        }
    }
}

Logger::~Logger() {
    if (logFileStream.is_open()) {
        logFileStream << TimeUtils::formatTimestamp(TimeUtils::getCurrentTimestamp())
                      << " [INFO] Logger shutting down." << std::endl;
        logFileStream.close();
    }
}

// getInstance: Khởi tạo instance static với giá trị từ AppConfig nếu path rỗng
Logger& Logger::getInstance(const std::string& logFilePath, LogLevel consoleLevel, LogLevel fileLevel, bool consoleLogging) {
    // Nếu logFilePath rỗng khi gọi lần đầu, dùng default từ AppConfig
    static std::string actualLogFilePath = logFilePath.empty() ? 
                                           (std::string(AppConfig::LOG_DIRECTORY) + AppConfig::LOG_FILENAME) : 
                                           logFilePath;
    // Tương tự cho các level nếu chúng được truyền vào với giá trị "đánh dấu"
    // Tuy nhiên, các enum không có giá trị "rỗng", nên ta sẽ dùng giá trị từ AppConfig nếu
    // các tham số mặc định trong .hpp không được thay đổi khi gọi từ main.
    // Trong trường hợp này, main.cpp sẽ gọi Logger::getInstance với các giá trị từ AppConfig.
    // Nếu gọi Logger::getInstance() không tham số, nó sẽ dùng các default cơ bản từ .hpp,
    // nhưng instance static sẽ được tạo một lần duy nhất.
    // Cách tốt nhất là main.cpp gọi 1 lần duy nhất với AppConfig.
    // Hoặc, làm cho hàm này phức tạp hơn để xử lý việc gọi lại với các config khác nhau (không khuyến khích cho singleton đơn giản).

    // Cách đơn giản: static instance sẽ được khởi tạo với các giá trị default từ AppConfig MỘT LẦN DUY NHẤT.
    // Các lần gọi getInstance() sau sẽ trả về cùng instance đó.
    // Để điều này hoạt động như mong đợi, lời gọi đầu tiên (ví dụ từ main.cpp) NÊN cung cấp các giá trị từ AppConfig.
    // Hoặc, chúng ta làm như sau:
    static Logger instance(
        logFilePath.empty() ? (std::string(AppConfig::LOG_DIRECTORY) + AppConfig::LOG_FILENAME) : logFilePath,
        consoleLevel, // Sẽ lấy giá trị từ lời gọi đầu tiên, main.cpp sẽ truyền AppConfig
        fileLevel,    // Tương tự
        consoleLogging // Tương tự
    );
    return instance;
}

// ... (phần còn lại của Logger.cpp không thay đổi: logLevelToString, log, debug, info, warning, error, setLogLevels, enableConsoleOutput)
// Đảm bảo chúng không hardcode giá trị mà dùng currentConsoleLogLevel, currentFileLogLevel
std::string Logger::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex); 

    std::string formattedMessage = TimeUtils::formatTimestamp(TimeUtils::getCurrentTimestamp()) +
                                   " [" + logLevelToString(level) + "] " + message;

    if (consoleOutputEnabled && static_cast<int>(level) >= static_cast<int>(currentConsoleLogLevel)) {
        if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }

    if (logFileStream.is_open() && static_cast<int>(level) >= static_cast<int>(currentFileLogLevel)) {
        logFileStream << formattedMessage << std::endl;
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