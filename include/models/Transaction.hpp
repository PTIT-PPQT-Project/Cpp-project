// include/models/Transaction.hpp
#pragma once

#include <string>
#include <ctime>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

// Enum for transaction statuses
enum class TransactionStatus {
    Pending,
    Completed,
    Failed,
    Cancelled
};

class Transaction {
public:
    std::string transactionId;      // Unique identifier for the transaction
    std::string sourceWalletId;     // ID of the wallet sending the points
    std::string targetWalletId;     // ID of the wallet receiving the points
    double amount;                  // Amount of points transferred
    std::string description;         // Optional description for the transaction
    time_t timestamp;               // Timestamp when the transaction was recorded/processed
    TransactionStatus status;        // Current status of the transaction

    // Default constructor
    Transaction();

    // Static utility functions for enum conversion
    static std::string statusToString(TransactionStatus status);
    static TransactionStatus stringToStatus(const std::string& statusStr);

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Transaction, transactionId, sourceWalletId, targetWalletId, amount, description, timestamp, status)

    // Custom JSON conversion for enums
    static void to_json(json& j, const TransactionStatus& status) {
        j = static_cast<int>(status);
}

    static void from_json(const json& j, TransactionStatus& status) {
    if (j.is_string()) {
            std::string statusStr = j.get<std::string>();
            if (statusStr == "Pending") status = TransactionStatus::Pending;
            else if (statusStr == "Completed") status = TransactionStatus::Completed;
            else if (statusStr == "Failed") status = TransactionStatus::Failed;
            else if (statusStr == "Cancelled") status = TransactionStatus::Cancelled;
            else throw std::runtime_error("Invalid TransactionStatus string value: " + statusStr);
        } else if (j.is_number()) {
            int statusInt = j.get<int>();
            if (statusInt == 0) status = TransactionStatus::Pending;
            else if (statusInt == 1) status = TransactionStatus::Completed;
            else if (statusInt == 2) status = TransactionStatus::Failed;
            else if (statusInt == 3) status = TransactionStatus::Cancelled;
            else throw std::runtime_error("Invalid TransactionStatus integer value: " + std::to_string(statusInt));
    } else {
            throw std::runtime_error("TransactionStatus must be a string or integer in JSON");
        }
    }
};