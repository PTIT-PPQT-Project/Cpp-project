// src/models/User.cpp
#include "../include/models/User.hpp" // Adjust path to be relative to source/models directory
#include <stdexcept> // For std::invalid_argument

// Default constructor
User::User() :
    userId(""),
    username(""),
    passwordHash(""),
    fullName(""),
    email(""),
    phoneNumber(""),
    role(UserRole::RegularUser),
    status(AccountStatus::NotActivated),
    otpSecretKey(""),
    isTemporaryPassword(false) {}

// Static utility function implementations
std::string User::roleToString(UserRole role) {
    switch (role) {
        case UserRole::RegularUser: return "RegularUser";
        case UserRole::AdminUser: return "AdminUser";
        default: return "Unknown";
    }
}

UserRole User::stringToRole(const std::string& roleStr) {
    if (roleStr == "RegularUser") return UserRole::RegularUser;
    else if (roleStr == "AdminUser") return UserRole::AdminUser;
    else if (roleStr == "Admin") return UserRole::AdminUser; // Handle legacy format
    else throw std::runtime_error("Invalid UserRole string value: " + roleStr);
}

std::string User::statusToString(AccountStatus status) {
    switch (status) {
        case AccountStatus::NotActivated: return "NotActivated";
        case AccountStatus::Active: return "Active";
        case AccountStatus::Inactive: return "Inactive";
        default: return "Unknown";
    }
}

AccountStatus User::stringToStatus(const std::string& statusStr) {
    if (statusStr == "NotActivated") return AccountStatus::NotActivated;
    else if (statusStr == "Active") return AccountStatus::Active;
    else if (statusStr == "Inactive") return AccountStatus::Inactive;
    else throw std::runtime_error("Invalid AccountStatus string value: " + statusStr);
}