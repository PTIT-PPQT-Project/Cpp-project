// include/utils/InputValidator.hpp
#pragma once

#include <string>
#include <regex> 

class InputValidator {
public:
    static bool isNonEmpty(const std::string& input);
    static bool isValidUsername(const std::string& username);
    static bool isValidPassword(const std::string& password);
    static bool isValidEmail(const std::string& email);
    static bool isValidPhoneNumber(const std::string& phoneNumber);
    static bool isValidPositiveAmount(double amount);
    static bool isValidInteger(const std::string& input, int& outValue);
    static bool isValidDouble(const std::string& input, double& outValue);

private:
    static std::string trim(const std::string& str);
};