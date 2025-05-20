// include/models/ModelJsonSerialization.hpp (or at the bottom of relevant model .hpp files)
#pragma once

#include "nlohmann/json.hpp"
#include "models/User.hpp"    // Ensure paths are correct if this file is in a different location
#include "models/Wallet.hpp"
#include "models/Transaction.hpp"

// Use a shorter alias for nlohmann::json
using json = nlohmann::json;

// --- UserRole enum ---
inline void to_json(json& j, const UserRole& ur) {
    j = User::roleToString(ur); 
}
inline void from_json(const json& j, UserRole& ur) {
    if (j.is_string()) {
        ur = User::stringToRole(j.get<std::string>());
    } else {
        throw json::type_error::create(302, "UserRole must be a string in JSON", &j);
    }
}

// --- AccountStatus enum ---
inline void to_json(json& j, const AccountStatus& as) {
    j = User::statusToString(as); 
}
inline void from_json(const json& j, AccountStatus& as) {
    if (j.is_string()) {
        as = User::stringToStatus(j.get<std::string>());
    } else {
        throw json::type_error::create(302, "AccountStatus must be a string in JSON", &j);
    }
}

// --- User class ---
inline void to_json(json& j, const User& u) {
    j = json{
        {"userId", u.userId}, {"username", u.username},
        {"hashedPassword", u.hashedPassword}, {"passwordSalt", u.passwordSalt},
        {"fullName", u.fullName}, {"email", u.email}, {"phoneNumber", u.phoneNumber},
        {"role", u.role}, {"creationTimestamp", u.creationTimestamp},
        {"lastLoginTimestamp", u.lastLoginTimestamp}, {"status", u.status},
        {"isTemporaryPassword", u.isTemporaryPassword}, {"otpSecretKey", u.otpSecretKey}
    };
}
inline void from_json(const json& j, User& u) {
    j.at("userId").get_to(u.userId); j.at("username").get_to(u.username);
    j.at("hashedPassword").get_to(u.hashedPassword); j.at("passwordSalt").get_to(u.passwordSalt);
    j.at("fullName").get_to(u.fullName); j.at("email").get_to(u.email); j.at("phoneNumber").get_to(u.phoneNumber);
    j.at("role").get_to(u.role); j.at("creationTimestamp").get_to(u.creationTimestamp);
    j.at("lastLoginTimestamp").get_to(u.lastLoginTimestamp); j.at("status").get_to(u.status);
    j.at("isTemporaryPassword").get_to(u.isTemporaryPassword);
    u.otpSecretKey = j.value("otpSecretKey", ""); 
}

// --- Wallet class ---
inline void to_json(json& j, const Wallet& w) {
    j = json{
        {"walletId", w.walletId}, {"userId", w.userId}, {"balance", w.balance},
        {"creationTimestamp", w.creationTimestamp}, {"lastUpdateTimestamp", w.lastUpdateTimestamp}
    };
}
inline void from_json(const json& j, Wallet& w) {
    j.at("walletId").get_to(w.walletId); j.at("userId").get_to(w.userId); j.at("balance").get_to(w.balance);
    j.at("creationTimestamp").get_to(w.creationTimestamp); j.at("lastUpdateTimestamp").get_to(w.lastUpdateTimestamp);
}

// --- TransactionStatus enum ---
inline void to_json(json& j, const TransactionStatus& ts) {
    j = Transaction::statusToString(ts);
}
inline void from_json(const json& j, TransactionStatus& ts) {
    if (j.is_string()) {
        ts = Transaction::stringToStatus(j.get<std::string>());
    } else {
        throw json::type_error::create(302, "TransactionStatus must be a string in JSON", &j);
    }
}

// --- Transaction class ---
inline void to_json(json& j, const Transaction& t) {
    j = json{
        {"transactionId", t.transactionId}, {"senderWalletId", t.senderWalletId},
        {"receiverWalletId", t.receiverWalletId}, {"amountTransferred", t.amountTransferred},
        {"transactionTimestamp", t.transactionTimestamp}, {"status", t.status},
        {"description", t.description}
    };
}
inline void from_json(const json& j, Transaction& t) {
    j.at("transactionId").get_to(t.transactionId); j.at("senderWalletId").get_to(t.senderWalletId);
    j.at("receiverWalletId").get_to(t.receiverWalletId); j.at("amountTransferred").get_to(t.amountTransferred);
    j.at("transactionTimestamp").get_to(t.transactionTimestamp); j.at("status").get_to(t.status);
    t.description = j.value("description", "");
}