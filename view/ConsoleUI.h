#pragma once
#include <string>
class ConsoleUI {
public:
    static void showLoginMenu();
    static std::string getInput(const std::string& label);
    static void showMessage(const std::string& msg);
};
