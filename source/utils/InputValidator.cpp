// src/utils/InputValidator.cpp
#include "utils/InputValidator.hpp"
#include "Config.h" // <<< THÊM ĐỂ DÙNG AppConfig
#include <algorithm> 
#include <cctype>    

// trim function (không thay đổi)
std::string InputValidator::trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = str.find_first_not_of(WHITESPACE);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(WHITESPACE);
    return str.substr(start, (end - start + 1));
}

// isNonEmpty function (không thay đổi)
bool InputValidator::isNonEmpty(const std::string& input) {
    return !trim(input).empty();
}

bool InputValidator::isValidUsername(const std::string& username) {
    // Sử dụng AppConfig
    if (username.length() < AppConfig::MIN_USERNAME_LENGTH || username.length() > AppConfig::MAX_USERNAME_LENGTH) {
        return false;
    }
    return std::all_of(username.begin(), username.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    });
}

bool InputValidator::isValidPassword(const std::string& password) {
    // Sử dụng AppConfig
    if (password.length() < AppConfig::MIN_PASSWORD_LENGTH) {
        return false;
    }
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    const std::string specialChars = "!@#$%^&*()_+-=[]{};':\",./<>?";

    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else if (specialChars.find(c) != std::string::npos) hasSpecial = true;
    }
    // Quyết định chính sách mật khẩu ở đây, ví dụ:
    // return hasUpper && hasLower && hasDigit && hasSpecial; // Đầy đủ
    return hasUpper && hasLower && hasDigit; // Hoặc đơn giản hơn
}

// isValidEmail (không thay đổi, không dùng AppConfig trực tiếp)
bool InputValidator::isValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.?\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, pattern);
}

bool InputValidator::isValidPhoneNumber(const std::string& phoneNumber) {
    std::string number = trim(phoneNumber);
    if (number.empty()) return false;

    size_t startIndex = 0;
    if (number[0] == '+') {
        startIndex = 1;
    }
    std::string digitPart = number.substr(startIndex);
    // Sử dụng AppConfig
    if (digitPart.length() < AppConfig::MIN_PHONE_NUMBER_DIGITS || digitPart.length() > AppConfig::MAX_PHONE_NUMBER_DIGITS) {
        return false;
    }
    return std::all_of(digitPart.begin(), digitPart.end(), ::isdigit);
}

// isValidPositiveAmount (không thay đổi)
bool InputValidator::isValidPositiveAmount(double amount) {
    return amount > 0.0;
}

// isValidInteger (không thay đổi)
bool InputValidator::isValidInteger(const std::string& input, int& outValue) {
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return false;
    for (size_t i = 0; i < trimmedInput.length(); ++i) {
        if (i == 0 && (trimmedInput[i] == '+' || trimmedInput[i] == '-')) continue;
        if (!std::isdigit(static_cast<unsigned char>(trimmedInput[i]))) return false;
    }
    try {
        size_t pos;
        outValue = std::stoi(trimmedInput, &pos);
        return pos == trimmedInput.length();
    } catch (const std::invalid_argument&) { return false; }
      catch (const std::out_of_range&) { return false; }
}

// isValidDouble (không thay đổi)
bool InputValidator::isValidDouble(const std::string& input, double& outValue) {
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return false;
    bool decimalPointFound = false;
    for (size_t i = 0; i < trimmedInput.length(); ++i) {
        if (i == 0 && (trimmedInput[i] == '+' || trimmedInput[i] == '-')) continue;
        if (trimmedInput[i] == '.') {
            if (decimalPointFound) return false;
            decimalPointFound = true;
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(trimmedInput[i]))) return false;
    }
    try {
        size_t pos;
        outValue = std::stod(trimmedInput, &pos);
        return pos == trimmedInput.length();
    } catch (const std::invalid_argument&) { return false; }
      catch (const std::out_of_range&) { return false; }
}