#include <iostream>
#include <string>
#include "controller/user_controller.h"
#include "view/menu_view.h"
#include "db/user_repository.h"

int main() {
    const std::string USER_DATA_FILE = "users.dat";
    UserRepository userRepo(USER_DATA_FILE);
    UserController userController(userRepo);
    
    int choice = -1;
    std::string username, password, email;
    
    while (choice != 0) {
        MenuView::displayMainMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                MenuView::displayRegistrationForm();
                std::cin >> username;
                std::cout << "Enter password: ";
                std::cin >> password;
                std::cout << "Enter email: ";
                std::cin >> email;
                
                if (userController.registerUser(username, password, email)) {
                    MenuView::displayMessage("Registration successful!");
                } else {
                    MenuView::displayError("Registration failed. Please check your input.");
                }
                break;
                
            case 0:
                MenuView::displayExitMessage();
                break;
                
            default:
                MenuView::displayError("Invalid choice. Please try again.");
                break;
        }
    }
    
    return 0;
} 