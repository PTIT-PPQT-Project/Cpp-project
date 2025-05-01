#include "user_controller.h"
#include "../utils/password_hasher.h"
#include "../model/user.h"
#include <regex>

UserController::UserController(UserRepository& repo) : userRepo(repo) {}

bool UserController::isValidEmail(const std::string& email) {
    const std::regex pattern(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"
    );
    return std::regex_match(email, pattern);
}

bool UserController::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    if (username.empty() || password.empty() || email.empty()) {
        return false;
    }
    
    if (!isValidEmail(email)) {
        return false;
    }
    
    auto [passwordHash, salt] = PasswordHasher::hashPassword(password);
    
    int newId = userRepo.getNextId();
    User newUser(username, passwordHash, salt, email, newId);
    
    bool success = userRepo.addUser(newUser);
    
    if (success) {
        userRepo.incrementNextId();
    }
    
    return success;
} 