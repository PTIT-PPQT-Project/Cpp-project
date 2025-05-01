#include "user.h"
#include <chrono>
#include <iomanip>
#include <sstream>

User::User(const std::string& username, const std::string& passwordHash, 
         const std::string& salt, const std::string& email, int newId)
    : username(username), passwordHash(passwordHash), salt(salt),
      email(email), isActive(true), id(newId) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    // Use localtime_s instead of localtime
    std::tm localTime;
    localtime_s(&localTime, &time);
    
    // Format time as string
    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    createdAt = ss.str();
}

User::User(int id, const std::string& username, const std::string& passwordHash, 
         const std::string& salt, const std::string& email, 
         const std::string& createdAt, bool isActive)
    : id(id), username(username), passwordHash(passwordHash), salt(salt),
      email(email), createdAt(createdAt), isActive(isActive) {}

std::string User::serialize() const {
    return std::to_string(id) + "," + 
           username + "," + 
           passwordHash + "," + 
           salt + "," + 
           email + "," + 
           createdAt + "," + 
           (isActive ? "1" : "0");
} 