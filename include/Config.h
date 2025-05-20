// include/Config.h
#pragma once

#include <string>
#include "utils/Logger.hpp" // For LogLevel enum definition.
                            // Ensure this path is correct based on your Logger.hpp location.
                            // If Logger.hpp is in include/utils/Logger.hpp, and Config.h is in include/,
                            // then "utils/Logger.hpp" is correct.

namespace AppConfig {

    // --- Application Information ---
    // Can be used for display purposes, OTP issuer name, etc.
    constexpr const char* APPLICATION_NAME = "Reward System";
    constexpr const char* APPLICATION_VERSION = "1.0.0";

    // --- Data Storage Configuration ---
    // Base directory for all data files. Ensure it ends with a slash.
    constexpr const char* DATA_DIRECTORY = "data/";

    // Specific filenames for data
    constexpr const char* USERS_FILENAME = "users.json";
    constexpr const char* WALLETS_FILENAME = "wallets.json";
    constexpr const char* TRANSACTIONS_FILENAME = "transactions.json";
    constexpr const char* BACKUP_SUBDIRECTORY = "backup/"; // Subdirectory within DATA_DIRECTORY

    // --- Logging Configuration ---
    // Base directory for log files. Ensure it ends with a slash.
    constexpr const char* LOG_DIRECTORY = "logs/";

    // Default filename for the application log.
    constexpr const char* LOG_FILENAME = "app.log";

    // Default log levels for the logger instance.
    // Note: The LogLevel enum itself is defined in Logger.hpp
    constexpr LogLevel DEFAULT_CONSOLE_LOG_LEVEL = LogLevel::INFO;
    constexpr LogLevel DEFAULT_FILE_LOG_LEVEL = LogLevel::DEBUG;
    constexpr bool DEFAULT_CONSOLE_LOGGING_ENABLED = true;

    // --- OTP (One-Time Password) Configuration ---
    // Issuer name to be displayed in OTP authenticator apps (e.g., Google Authenticator, Authy).
    constexpr const char* OTP_ISSUER_NAME = APPLICATION_NAME; // Using APPLICATION_NAME for consistency

    // --- Security & Input Validation Parameters ---
    // Username policies
    constexpr int MIN_USERNAME_LENGTH = 3;
    constexpr int MAX_USERNAME_LENGTH = 20;

    // Password policies
    constexpr int MIN_PASSWORD_LENGTH = 8;
    // Conceptual flags for password complexity (actual logic is in InputValidator)
    // constexpr bool REQUIRE_PASSWORD_UPPERCASE = true;
    // constexpr bool REQUIRE_PASSWORD_LOWERCASE = true;
    // constexpr bool REQUIRE_PASSWORD_DIGIT = true;
    // constexpr bool REQUIRE_PASSWORD_SPECIAL_CHAR = true; // Already part of InputValidator logic

    // Phone number policies (digit count after optional '+')
    constexpr int MIN_PHONE_NUMBER_DIGITS = 9;
    constexpr int MAX_PHONE_NUMBER_DIGITS = 15;

    // --- Wallet & Transaction Configuration ---
    // Default balance for newly created user wallets
    constexpr double DEFAULT_INITIAL_WALLET_BALANCE = 0.0;

    // Special Wallet IDs (used for system operations like master funding or generic deposits)
    constexpr const char* MASTER_WALLET_ID = "MASTER_WLT_001";
    // ID for deposits that don't originate from a specific user or the master wallet (e.g., initial system points)
    constexpr const char* SYSTEM_WALLET_ID_FOR_DEPOSITS = "SYSTEM_DEPOSIT_SRC";

    // --- UI/UX Configuration (Optional) ---
    // Example: Could be used if you implement pagination for listings.
    // constexpr int ITEMS_PER_PAGE = 10;

} // namespace AppConfig