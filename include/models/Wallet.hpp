// include/models/Wallet.hpp
#pragma once

#include <string>
#include <ctime>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Wallet {
public:
    std::string walletId;           // Unique identifier for the wallet
    std::string userId;             // ID of the user who owns this wallet
    double balance;                 // Current point balance in the wallet
    time_t creationTimestamp;       // Timestamp of wallet creation
    time_t lastUpdateTimestamp;   // Timestamp of the last balance update

    // Default constructor
    Wallet();
};

// --- nlohmann/json serialization/deserialization for Wallet class ---
inline void to_json(json& j, const Wallet& w) {
    j = json{
        {"walletId", w.walletId},
        {"userId", w.userId},
        {"balance", w.balance},
        {"creationTimestamp", w.creationTimestamp},
        {"lastUpdateTimestamp", w.lastUpdateTimestamp}
    };
}

inline void from_json(const json& j, Wallet& w) {
    j.at("walletId").get_to(w.walletId);
    j.at("userId").get_to(w.userId);
    j.at("balance").get_to(w.balance);
    j.at("creationTimestamp").get_to(w.creationTimestamp);
    j.at("lastUpdateTimestamp").get_to(w.lastUpdateTimestamp);
}