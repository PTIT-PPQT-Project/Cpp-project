#include "../../include/utils/InputValidator.hpp"
#include <algorithm>
#include <regex>
#include <string>

namespace {
    // Helper function for common character checks
    bool isValidCharForUsername(char c) {
        return std::isalnum(c) || c == '_';
    }

    // Common special characters for password validation
    const std::string SPECIAL_CHARS = "!@#$%^&*()_+-=[]{};':\",./<>?";
}

bool InputValidator::isNonEmpty(const std::string& input) {
    return !input.empty();
}

bool InputValidator::isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    return std::all_of(username.begin(), username.end(), isValidCharForUsername);
}

bool InputValidator::isValidPassword(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (SPECIAL_CHARS.find(c) != std::string::npos) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

bool InputValidator::isValidEmail(const std::string& email) {
    // More permissive regex: allows letters, numbers, dots, hyphens, and plus signs before @,
    // followed by domain with at least one dot
    static const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return isNonEmpty(email) && std::regex_match(email, pattern);
}

bool InputValidator::isValidPhoneNumber(const std::string& phoneNumber) {
    if (!isNonEmpty(phoneNumber)) {
        return false;
    }

    // Remove common separators (spaces, dashes, parentheses)
    std::string cleaned;
    std::copy_if(phoneNumber.begin(), phoneNumber.end(), std::back_inserter(cleaned),
                 [](char c) { return std::isdigit(c) || c == '+'; });

    size_t start_index = 0;
    if (!cleaned.empty() && cleaned[0] == '+') {
        start_index = 1;
    }

    size_t digit_count = cleaned.length() - start_index;
    if (digit_count < 9 || digit_count > 15) {
        return false;
    }

    return std::all_of(cleaned.begin() + start_index, cleaned.end(), std::isdigit);
}

bool InputValidator::isValidPositiveAmount(double amount) {
    return amount > 0.0;
}

bool InputValidator::isValidInteger(const std::string& input, int& outValue) {
    if (!isNonEmpty(input)) {
        return false;
    }

    try {
        size_t pos;
        outValue = std::stoi(input, &pos);
        return pos == input.length();
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool InputValidator::isValidDouble(const std::string& input, double& outValue) {
    if (!isNonEmpty(input)) {
        return false;
    }

    try {
        size_t pos;
        outValue = std::stod(input, &pos);
        return pos == input.length();
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}