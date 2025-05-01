#pragma once
#include "../db/user_repository.h"
#include <string>

class UserController {
private:
    UserRepository& userRepo;
    static bool isValidEmail(const std::string& email);

public:
    UserController(UserRepository& repo);
    bool registerUser(const std::string& username, const std::string& password, const std::string& email);
}; 