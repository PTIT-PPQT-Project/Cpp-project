#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "User.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>

class UserManager {
private:
    static UserManager* instance;
    std::vector<std::shared_ptr<User>> users;
    std::string dataFile;
    
    UserManager(const std::string& filename = "users.csv");
    void loadUsers();
    void saveUsers();
    
public:
    static UserManager* getInstance(const std::string& filename = "users.csv");
    
    // User management
    bool registerUser(const std::string& username, const std::string& password,
                     const std::string& email, bool isAdmin = false);
    bool registerUserWithAutoPassword(const std::string& username,
                                    const std::string& email,
                                    bool isAdmin = false);
    std::shared_ptr<User> authenticateUser(const std::string& username,
                                         const std::string& password);
    
    // User operations
    bool changeUserPassword(const std::string& username,
                          const std::string& oldPassword,
                          const std::string& newPassword);
    bool updateUserInfo(const std::string& username,
                       const std::string& newEmail);
    
    // Admin operations
    std::vector<std::shared_ptr<User>> getAllUsers() const;
    bool deleteUser(const std::string& username);
    
    // Backup operations
    void createBackup();
    bool restoreFromBackup();
};

#endif 