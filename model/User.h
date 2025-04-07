#pragma once
#include <string>
class User {
public:
    int id;
    std::string username;
    std::string password_hash;
    std::string role;
    static bool createUser(const std::string& username, const std::string& password);
    static User* authenticate(const std::string& username, const std::string& password);
};
