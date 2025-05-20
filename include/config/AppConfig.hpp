#pragma once

namespace AppConfig {
    // Password requirements
    constexpr int MIN_PASSWORD_LENGTH = 8;
    constexpr int MAX_PASSWORD_LENGTH = 128;
    
    // Username requirements
    constexpr int MIN_USERNAME_LENGTH = 3;
    constexpr int MAX_USERNAME_LENGTH = 32;
    
    // Email requirements
    constexpr int MAX_EMAIL_LENGTH = 254;  // RFC 5321
    
    // Phone number requirements
    constexpr int MIN_PHONE_LENGTH = 10;
    constexpr int MAX_PHONE_LENGTH = 15;
    
    // File paths
    constexpr const char* USERS_FILE_PATH = "data/users.json";
    constexpr const char* WALLETS_FILE_PATH = "data/wallets.json";
    constexpr const char* TRANSACTIONS_FILE_PATH = "data/transactions.json";
    constexpr const char* LOG_FILE_PATH = "logs/app.log";
    
    // Session settings
    constexpr int SESSION_TIMEOUT_MINUTES = 30;
    
    // OTP settings
    constexpr int OTP_SECRET_KEY_LENGTH = 32;
    constexpr int OTP_CODE_LENGTH = 6;
    constexpr int OTP_VALIDITY_SECONDS = 30;
    
    // Transaction limits
    constexpr double MIN_TRANSACTION_AMOUNT = 0.01;
    constexpr double MAX_TRANSACTION_AMOUNT = 10000.00;
    
    // Wallet limits
    constexpr double MIN_WALLET_BALANCE = 0.00;
    constexpr double MAX_WALLET_BALANCE = 1000000.00;
} 