// src/models/User.cpp
#include "include/models/User.hpp" // Điều chỉnh đường dẫn nếu cần
#include <stdexcept> // For std::invalid_argument

// Default constructor
User::User() :
    userId(""),
    username(""),
    hashedPassword(""),
    passwordSalt(""),
    fullName(""),
    email(""),
    phoneNumber(""),
    role(UserRole::RegularUser), // Default role
    creationTimestamp(0),
    lastLoginTimestamp(0),
    status(AccountStatus::NotActivated), // Default status
    isTemporaryPassword(false),
    otpSecretKey("") {}

// Static utility function implementations
std::string User::roleToString(UserRole r) {
    switch (r) {
        case UserRole::RegularUser: return "RegularUser";
        case UserRole::AdminUser: return "AdminUser";
        default: return "UnknownRole";
    }
}

UserRole User::stringToRole(const std::string& roleStr) {
    if (roleStr == "RegularUser") return UserRole::RegularUser;
    if (roleStr == "AdminUser") return UserRole::AdminUser;
    // For robustness, you might log an error and return a default role
    // or simply throw an exception if the string is unexpected.
    throw std::invalid_argument("Invalid role string provided for conversion: " + roleStr);
}

std::string User::statusToString(AccountStatus s) {
    switch (s) {
        case AccountStatus::Active: return "Active";
        case AccountStatus::Inactive: return "Inactive";
        case AccountStatus::NotActivated: return "NotActivated";
        default: return "UnknownStatus";
    }
}

AccountStatus User::stringToStatus(const std::string& statusStr) {
    if (statusStr == "Active") return AccountStatus::Active;
    if (statusStr == "Inactive") return AccountStatus::Inactive;
    if (statusStr == "NotActivated") return AccountStatus::NotActivated;
    throw std::invalid_argument("Invalid status string provided for conversion: " + statusStr);
}