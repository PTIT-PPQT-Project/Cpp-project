#include "AuthController.h"
#include "../view/ConsoleUI.h"
#include "../model/User.h"
#include <iostream>
void AuthController::showMenu() {
    while (true) {
        ConsoleUI::showLoginMenu();
        std::string option = ConsoleUI::getInput("Choose option (1-Login, 2-Register, 0-Exit): ");
        if (option == "1") login();
        else if (option == "2") registerUser();
        else break;
    }
}
void AuthController::login() {
    std::string username = ConsoleUI::getInput("Username");
    std::string password = ConsoleUI::getInput("Password");
    User* user = User::authenticate(username, password);
    if (user) {
        ConsoleUI::showMessage("Login successful!");
    } else {
        ConsoleUI::showMessage("Login failed!");
    }
}
void AuthController::registerUser() {
    std::string username = ConsoleUI::getInput("New Username");
    std::string password = ConsoleUI::getInput("New Password");
    if (User::createUser(username, password)) {
        ConsoleUI::showMessage("Registration successful!");
    } else {
        ConsoleUI::showMessage("Registration failed!");
    }
}
