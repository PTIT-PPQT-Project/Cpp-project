// include/models/Transaction.hpp
#pragma once

#include <string>
#include <ctime>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Enum for transaction statuses
enum class TransactionStatus {
    PendingOtpConfirmation,
    Successful,
    FailedInsufficientFunds,
    FailedOtpInvalid,
    FailedWalletNotFound,
    FailedSystemError,
    CancelledByUser // Or just "Cancelled"
};

class Transaction {
public:
    std::string transactionId;      // Unique identifier for the transaction
    std::string senderWalletId;     // ID of the wallet sending the points
    std::string receiverWalletId;   // ID of the wallet receiving the points
    double amountTransferred;       // Amount of points transferred
    time_t transactionTimestamp;    // Timestamp when the transaction was recorded/processed
    TransactionStatus status;       // Current status of the transaction
    std::string description;        // Optional description for the transaction

    // Default constructor
    Transaction();

    // Static utility functions for enum conversion
    static std::string statusToString(TransactionStatus s);
    static TransactionStatus stringToStatus(const std::string& statusStr);
};

// --- nlohmann/json serialization/deserialization for Enums ---
// TransactionStatus
inline void to_json(json& j, const TransactionStatus& ts) {
    j = Transaction::statusToString(ts);
}
inline void from_json(const json& j, TransactionStatus& ts) {
    if (j.is_string()) {
        ts = Transaction::stringToStatus(j.get<std::string>());
    } else {
        throw json::type_error::create(302, "TransactionStatus must be a string in JSON", j);
    }
}

// --- nlohmann/json serialization/deserialization for Transaction class ---
inline void to_json(json& j, const Transaction& t) {
    j = json{
        {"transactionId", t.transactionId},
        {"senderWalletId", t.senderWalletId},
        {"receiverWalletId", t.receiverWalletId},
        {"amountTransferred", t.amountTransferred},
        {"transactionTimestamp", t.transactionTimestamp},
        {"status", t.status}, // Automatically uses to_json for TransactionStatus
        {"description", t.description}
    };
}

inline void from_json(const json& j, Transaction& t) {
    j.at("transactionId").get_to(t.transactionId);
    j.at("senderWalletId").get_to(t.senderWalletId);
    j.at("receiverWalletId").get_to(t.receiverWalletId);
    j.at("amountTransferred").get_to(t.amountTransferred);
    j.at("transactionTimestamp").get_to(t.transactionTimestamp);
    j.at("status").get_to(t.status); // Automatically uses from_json for TransactionStatus
    t.description = j.value("description", ""); // Gracefully handle if key is missing
}