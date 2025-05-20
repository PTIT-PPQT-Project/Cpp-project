// include/models/User.hpp
#pragma once

#include <string>
#include <ctime>
#include <vector> // Thường không cần trực tiếp trong model User, nhưng có thể cần cho các hàm tiện ích khác
#include "nlohmann/json.hpp" // Đảm bảo bạn đã có thư viện nlohmann/json trong include path

// Sử dụng alias cho nlohmann::json
using json = nlohmann::json;

// Enum for user roles
enum class UserRole {
    RegularUser,
    AdminUser
};

// Enum for account statuses
enum class AccountStatus {
    Active,
    Inactive,
    NotActivated // Default status or for accounts pending activation
};

class User {
public:
    std::string userId;             // Unique identifier for the user (e.g., UUID)
    std::string username;           // Unique login name
    std::string hashedPassword;     // Hashed password
    std::string passwordSalt;       // Salt used for hashing the password
    std::string fullName;           // Full name of the user
    std::string email;              // Unique email address
    std::string phoneNumber;        // Phone number
    UserRole role;                  // Role of the user in the system
    time_t creationTimestamp;       // Timestamp of account creation
    time_t lastLoginTimestamp;    // Timestamp of the last login
    AccountStatus status;           // Current status of the account
    bool isTemporaryPassword;       // True if the password was auto-generated and not yet changed
    std::string otpSecretKey;       // Secret key (Base32 encoded) for OTP generation

    // Default constructor
    User();

    // Static utility functions for enum conversion (useful for display or other logic)
    static std::string roleToString(UserRole r);
    static UserRole stringToRole(const std::string& roleStr);
    static std::string statusToString(AccountStatus s);
    static AccountStatus stringToStatus(const std::string& statusStr);
};

// --- nlohmann/json serialization/deserialization for Enums ---
// UserRole
inline void to_json(json& j, const UserRole& ur) {
    j = User::roleToString(ur);
}
inline void from_json(const json& j, UserRole& ur) {
    if (j.is_string()) {
        ur = User::stringToRole(j.get<std::string>());
    } else {
        // Handle error or default: for now, throw or assign a default.
        // For robustness, you might want to log an error and assign a default.
        throw json::type_error::create(302, "UserRole must be a string in JSON", &j);
    }
}

// AccountStatus
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

// --- nlohmann/json serialization/deserialization for User class ---
inline void to_json(json& j, const User& u) {
    j = json{
        {"userId", u.userId},
        {"username", u.username},
        {"hashedPassword", u.hashedPassword},
        {"passwordSalt", u.passwordSalt},
        {"fullName", u.fullName},
        {"email", u.email},
        {"phoneNumber", u.phoneNumber},
        {"role", u.role}, // Automatically uses to_json for UserRole
        {"creationTimestamp", u.creationTimestamp},
        {"lastLoginTimestamp", u.lastLoginTimestamp},
        {"status", u.status}, // Automatically uses to_json for AccountStatus
        {"isTemporaryPassword", u.isTemporaryPassword},
        {"otpSecretKey", u.otpSecretKey}
    };
}

inline void from_json(const json& j, User& u) {
    j.at("userId").get_to(u.userId);
    j.at("username").get_to(u.username);
    j.at("hashedPassword").get_to(u.hashedPassword);
    j.at("passwordSalt").get_to(u.passwordSalt);
    j.at("fullName").get_to(u.fullName);
    j.at("email").get_to(u.email);
    j.at("phoneNumber").get_to(u.phoneNumber);
    j.at("role").get_to(u.role); // Automatically uses from_json for UserRole
    j.at("creationTimestamp").get_to(u.creationTimestamp);
    j.at("lastLoginTimestamp").get_to(u.lastLoginTimestamp);
    j.at("status").get_to(u.status); // Automatically uses from_json for AccountStatus
    j.at("isTemporaryPassword").get_to(u.isTemporaryPassword);
    u.otpSecretKey = j.value("otpSecretKey", ""); // Gracefully handle if key is missing
}