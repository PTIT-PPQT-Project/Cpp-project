#include "menu_view.h"
#include <iostream>

void MenuView::displayMainMenu() {
    std::cout << "\n===== USER REGISTRATION SYSTEM =====\n";
    std::cout << "1. Register New User\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";
}

void MenuView::displayRegistrationForm() {
    std::cout << "\n===== USER REGISTRATION =====\n";
    std::cout << "Enter username: ";
}

void MenuView::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void MenuView::displayError(const std::string& error) {
    std::cout << "Error: " << error << std::endl;
}

void MenuView::displayExitMessage() {
    std::cout << "Exiting...\n";
    std::cout << "\nThank you!\n";
} 