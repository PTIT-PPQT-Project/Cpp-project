#pragma once
#include <string>

class MenuView {
public:
    static void displayMainMenu();
    static void displayRegistrationForm();
    static void displayMessage(const std::string& message);
    static void displayError(const std::string& error);
    static void displayExitMessage();
}; 