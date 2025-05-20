// src/models/Transaction.cpp
#include "../include/models/Transaction.hpp" // Điều chỉnh đường dẫn nếu cần
#include <stdexcept> // For std::invalid_argument

// Default constructor
Transaction::Transaction() :
    amount(0.0), 
    timestamp(time(nullptr)), 
    status(TransactionStatus::Pending) {}

// Static utility function implementations
std::string Transaction::statusToString(TransactionStatus status) {
    switch (status) {
        case TransactionStatus::Pending: return "Pending";
        case TransactionStatus::Completed: return "Completed";
        case TransactionStatus::Failed: return "Failed";
        case TransactionStatus::Cancelled: return "Cancelled";
        default: return "Unknown";
    }
}

TransactionStatus Transaction::stringToStatus(const std::string& statusStr) {
    if (statusStr == "Pending") return TransactionStatus::Pending;
    else if (statusStr == "Completed") return TransactionStatus::Completed;
    else if (statusStr == "Failed") return TransactionStatus::Failed;
    else if (statusStr == "Cancelled") return TransactionStatus::Cancelled;
    else throw std::runtime_error("Invalid TransactionStatus string value: " + statusStr);
}