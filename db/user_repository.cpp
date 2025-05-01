#include "user_repository.h"
#include <fstream>
#include <sstream>

UserRepository::UserRepository(const std::string& filePath) : filePath(filePath) {
    loadUsers();
}

UserRepository::~UserRepository() {
    saveUsers();
}

void UserRepository::loadUsers() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        nextId = 1;
        return;
    }
    
    std::string line;
    int maxId = 0;
    
    while (std::getline(file, line)) {
        size_t pos = 0;
        std::vector<std::string> tokens;
        std::string token;
        
        while ((pos = line.find(",")) != std::string::npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos + 1);
        }
        tokens.push_back(line);
        
        if (tokens.size() == 7) {
            int id = std::stoi(tokens[0]);
            if (id > maxId) {
                maxId = id;
            }
            
            std::string username = tokens[1];
            std::string passwordHash = tokens[2];
            std::string salt = tokens[3];
            std::string email = tokens[4];
            std::string createdAt = tokens[5];
            bool isActive = (tokens[6] == "1");
            
            users.emplace_back(id, username, passwordHash, salt, email, createdAt, isActive);
        }
    }
    
    nextId = maxId + 1;
    file.close();
}

void UserRepository::saveUsers() {
    std::ofstream file(filePath);
    if (file.is_open()) {
        for (const auto& user : users) {
            file << user.serialize() << std::endl;
        }
        file.close();
    }
}

bool UserRepository::addUser(const User& user) {
    for (const auto& existingUser : users) {
        if (existingUser.getUsername() == user.getUsername()) {
            return false;
        }
    }
    
    users.push_back(user);
    saveUsers();
    return true;
}

User* UserRepository::getUserByUsername(const std::string& username) {
    for (auto& user : users) {
        if (user.getUsername() == username) {
            return &user;
        }
    }
    return nullptr;
}

int UserRepository::getNextId() const {
    return nextId;
}

void UserRepository::incrementNextId() {
    nextId++;
} 