#include "ConsoleUI.h"
#include <iostream>
void ConsoleUI::showLoginMenu() {
    std::cout << "\n=== Wallet App ===\n";
    std::cout << "1. Login\n";
    std::cout << "2. Register\n";
    std::cout << "0. Exit\n";
}
std::string ConsoleUI::getInput(const std::string& label) {
    std::cout << label << ": ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}
void ConsoleUI::showMessage(const std::string& msg) {
    std::cout << msg << "\n";
}
