// include/models/User.hpp
#pragma once

#include <string>
#include <ctime>
#include <vector> // Thường không cần trực tiếp trong model User, nhưng có thể cần cho các hàm tiện ích khác
#include "nlohmann/json.hpp" // Đảm bảo bạn đã có thư viện nlohmann/json trong include path
#include <optional>

// Sử dụng alias cho nlohmann::json
using json = nlohmann::json;

// Enum for user roles
enum class UserRole {
    RegularUser,
    AdminUser
};

// Enum for account statuses
enum class AccountStatus {
    NotActivated,
    Active,
    Inactive
};

// JSON serialization for enums
inline void to_json(json& j, const UserRole& role) {
    switch (role) {
        case UserRole::RegularUser: j = "RegularUser"; break;
        case UserRole::AdminUser: j = "AdminUser"; break;
        default: j = "Unknown";
    }
}

inline void from_json(const json& j, UserRole& role) {
    std::string roleStr = j.get<std::string>();
    if (roleStr == "RegularUser") role = UserRole::RegularUser;
    else if (roleStr == "AdminUser") role = UserRole::AdminUser;
    else role = UserRole::RegularUser; // Default to RegularUser for unknown values
}

inline void to_json(json& j, const AccountStatus& status) {
    switch (status) {
        case AccountStatus::NotActivated: j = "NotActivated"; break;
        case AccountStatus::Active: j = "Active"; break;
        case AccountStatus::Inactive: j = "Inactive"; break;
        default: j = "Unknown";
    }
}

inline void from_json(const json& j, AccountStatus& status) {
    std::string statusStr = j.get<std::string>();
    if (statusStr == "NotActivated") status = AccountStatus::NotActivated;
    else if (statusStr == "Active") status = AccountStatus::Active;
    else if (statusStr == "Inactive") status = AccountStatus::Inactive;
    else status = AccountStatus::NotActivated; // Default to NotActivated for unknown values
}

class User {
public:
    std::string userId;             // Unique identifier for the user (e.g., UUID)
    std::string username;           // Unique login name
    std::string passwordHash;       // Hashed password
    std::string fullName;           // Full name of the user
    std::string email;              // Unique email address
    std::string phoneNumber;        // Phone number
    UserRole role;                  // Role of the user in the system
    AccountStatus status;           // Current status of the account
    std::string otpSecretKey;       // Secret key (Base32 encoded) for OTP generation
    bool isTemporaryPassword;       // True if the password was auto-generated and not yet changed

    // Default constructor
    User();

    // Static utility functions for enum conversion (useful for display or other logic)
    static std::string roleToString(UserRole role);
    static UserRole stringToRole(const std::string& roleStr);
    static std::string statusToString(AccountStatus status);
    static AccountStatus stringToStatus(const std::string& statusStr);

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, userId, username, passwordHash, fullName, email, phoneNumber, role, status, otpSecretKey, isTemporaryPassword)
};