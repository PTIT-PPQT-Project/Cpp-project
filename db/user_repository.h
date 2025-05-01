#pragma once
#include "../model/user.h"
#include <string>
#include <vector>
#include <fstream>

class UserRepository {
private:
    std::string filePath;
    std::vector<User> users;
    int nextId;
    
    void loadUsers();
    void saveUsers();
    
public:
    UserRepository(const std::string& filePath);
    ~UserRepository();
    
    bool addUser(const User& user);
    User* getUserByUsername(const std::string& username);
    int getNextId() const;
    void incrementNextId();
}; 