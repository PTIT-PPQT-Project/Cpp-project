// src/utils/InputValidator.cpp
#include "../../include/utils/InputValidator.hpp"
#include <algorithm> // For std::all_of, std::any_of
#include <cctype>    // For std::isspace, std::isalnum, etc.

std::string InputValidator::trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = str.find_first_not_of(WHITESPACE);
    if (start == std::string::npos) {
        return ""; // String contains only whitespace
    }
    size_t end = str.find_last_not_of(WHITESPACE);
    return str.substr(start, (end - start + 1));
}

bool InputValidator::isNonEmpty(const std::string& input) {
    return !trim(input).empty();
}

bool InputValidator::isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    // Allow alphanumeric characters and underscore
    return std::all_of(username.begin(), username.end(), [](char c) {
        return std::isalnum(c) || c == '_';
    });
}

bool InputValidator::isValidPassword(const std::string& password) {
    if (password.length() < 8) {
        return false; // Minimum length
    }
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    const std::string specialChars = "!@#$%^&*()_+-=[]{};':\",./<>?";

    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (specialChars.find(c) != std::string::npos) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

bool InputValidator::isValidEmail(const std::string& email) {
    // Basic regex for email validation. More comprehensive regex can be very complex.
    // This one checks for: something@something.something
    // \w matches alphanumeric characters and underscore.
    // For stricter validation, consider a dedicated email validation library or a more robust regex.
    const std::regex pattern(R"((\w+)(\.?\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, pattern);
}

bool InputValidator::isValidPhoneNumber(const std::string& phoneNumber) {
    std::string number = trim(phoneNumber);
    if (number.empty()) return false;

    size_t start_index = 0;
    if (number[0] == '+') {
        start_index = 1;
    }

    if (number.length() - start_index < 9 || number.length() - start_index > 15) { // e.g. 9-15 digits
        return false;
    }

    return std::all_of(number.begin() + start_index, number.end(), ::isdigit);
}

bool InputValidator::isValidPositiveAmount(double amount) {
    return amount > 0.0;
}

bool InputValidator::isValidInteger(const std::string& input, int& outValue) {
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return false;
    try {
        size_t pos;
        outValue = std::stoi(trimmedInput, &pos);
        // Check if the entire string was consumed by stoi
        return pos == trimmedInput.length();
    } catch (const std::invalid_argument& ia) {
        return false;
    } catch (const std::out_of_range& oor) {
        return false;
    }
}

bool InputValidator::isValidDouble(const std::string& input, double& outValue) {
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return false;
    try {
        size_t pos;
        outValue = std::stod(trimmedInput, &pos);
        // Check if the entire string was consumed by stod
        return pos == trimmedInput.length();
    } catch (const std::invalid_argument& ia) {
        return false;
    } catch (const std::out_of_range& oor) {
        return false;
    }
}