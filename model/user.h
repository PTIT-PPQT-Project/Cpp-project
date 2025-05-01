#pragma once
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class User {
private:
    int id;
    std::string username;
    std::string passwordHash;
    std::string salt; 
    std::string email;
    std::string createdAt;
    bool isActive;

public:
    User(const std::string& username, const std::string& passwordHash, 
         const std::string& salt, const std::string& email, int newId);
    
    User(int id, const std::string& username, const std::string& passwordHash, 
         const std::string& salt, const std::string& email, 
         const std::string& createdAt, bool isActive);
          
    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getPasswordHash() const { return passwordHash; }
    std::string getSalt() const { return salt; }
    std::string getEmail() const { return email; }
    std::string getCreatedAt() const { return createdAt; }
    bool getIsActive() const { return isActive; }
    
    std::string serialize() const;
}; 