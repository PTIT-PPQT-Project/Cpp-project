// src/models/Transaction.cpp
#include "../../include/models/Transaction.hpp" // Fixed include path
#include <stdexcept> // For std::invalid_argument

// Default constructor
Transaction::Transaction() :
    transactionId(""),
    senderWalletId(""),
    receiverWalletId(""),
    amountTransferred(0.0),
    transactionTimestamp(0),
    status(TransactionStatus::PendingOtpConfirmation), // Example default
    description("") {}

// Static utility function implementations
std::string Transaction::statusToString(TransactionStatus s) {
    switch (s) {
        case TransactionStatus::PendingOtpConfirmation: return "PendingOtpConfirmation";
        case TransactionStatus::Successful: return "Successful";
        case TransactionStatus::FailedInsufficientFunds: return "FailedInsufficientFunds";
        case TransactionStatus::FailedOtpInvalid: return "FailedOtpInvalid";
        case TransactionStatus::FailedWalletNotFound: return "FailedWalletNotFound";
        case TransactionStatus::FailedSystemError: return "FailedSystemError";
        case TransactionStatus::CancelledByUser: return "CancelledByUser";
        default: return "UnknownStatus";
    }
}

TransactionStatus Transaction::stringToStatus(const std::string& statusStr) {
    if (statusStr == "PendingOtpConfirmation") return TransactionStatus::PendingOtpConfirmation;
    if (statusStr == "Successful") return TransactionStatus::Successful;
    if (statusStr == "FailedInsufficientFunds") return TransactionStatus::FailedInsufficientFunds;
    if (statusStr == "FailedOtpInvalid") return TransactionStatus::FailedOtpInvalid;
    if (statusStr == "FailedWalletNotFound") return TransactionStatus::FailedWalletNotFound;
    if (statusStr == "FailedSystemError") return TransactionStatus::FailedSystemError;
    if (statusStr == "CancelledByUser") return TransactionStatus::CancelledByUser;
    throw std::invalid_argument("Invalid transaction status string provided for conversion: " + statusStr);
}