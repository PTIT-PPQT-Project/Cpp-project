// include/Config.h
#pragma once

#include <string>
#include "utils/Logger.hpp" // Fixed include path

namespace AppConfig {

    // === Data File Configuration ===
    // Directory where data files (users.json, wallets.json, etc.) are stored.
    // Ensure it ends with a slash.
    constexpr const char* DATA_DIRECTORY = "data/";

    // Filenames for data storage
    constexpr const char* USERS_FILENAME = "users.json";
    constexpr const char* WALLETS_FILENAME = "wallets.json";
    constexpr const char* TRANSACTIONS_FILENAME = "transactions.json";
    constexpr const char* BACKUP_SUBDIRECTORY = "backup/"; // Subdirectory within DATA_DIRECTORY for backups

    // === Logging Configuration ===
    // Directory where log files are stored.
    // Ensure it ends with a slash.
    constexpr const char* LOG_DIRECTORY = "logs/";

    // Default filename for the application log.
    constexpr const char* LOG_FILENAME = "app.log";

    // Default log levels for the logger instance.
    // Note: LogLevel enum is defined in Logger.hpp
    constexpr LogLevel DEFAULT_CONSOLE_LOG_LEVEL = LogLevel::INFO;
    constexpr LogLevel DEFAULT_FILE_LOG_LEVEL = LogLevel::DEBUG;
    constexpr bool DEFAULT_CONSOLE_LOGGING_ENABLED = true;

    // === OTP Configuration ===
    // Issuer name to be displayed in OTP authenticator apps.
    constexpr const char* OTP_ISSUER_NAME = "RewardSystemApp";

    // === Security & Validation Configuration ===
    // Username policies
    constexpr int MIN_USERNAME_LENGTH = 3;
    constexpr int MAX_USERNAME_LENGTH = 20;

    // Password policies
    constexpr int MIN_PASSWORD_LENGTH = 8;
    // Flags for password complexity (can be used by InputValidator)
    // These are more conceptual for Config.h; actual checks are in InputValidator.
    // constexpr bool REQUIRE_PASSWORD_UPPERCASE = true;
    // constexpr bool REQUIRE_PASSWORD_LOWERCASE = true;
    // constexpr bool REQUIRE_PASSWORD_DIGIT = true;
    // constexpr bool REQUIRE_PASSWORD_SPECIAL_CHAR = true;

    // Phone number policies
    constexpr int MIN_PHONE_NUMBER_DIGITS = 9;
    constexpr int MAX_PHONE_NUMBER_DIGITS = 15;


    // === Application Information ===
    // (Can be used for display purposes or User-Agent strings if making network requests)
    constexpr const char* APPLICATION_NAME = "Reward System";
    constexpr const char* APPLICATION_VERSION = "1.0.0";


    // === Wallet Configuration ===
    // Default balance for newly created wallets
    constexpr double DEFAULT_INITIAL_WALLET_BALANCE = 0.0;

    // Special Wallet IDs
    constexpr const char* MASTER_WALLET_ID = "MASTER_WALLET_001";
    constexpr const char* SYSTEM_WALLET_ID_FOR_DEPOSITS = "SYSTEM_DEPOSIT_SRC"; // For deposits not from master


} // namespace AppConfig