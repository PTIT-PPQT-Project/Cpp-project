#include "UserManager.h"
#include "OTPManager.h"
#include "Wallet.h"
#include <iostream>
#include <string>

void printMenu() {
    std::cout << "\n=== Wallet Management System ===\n"
              << "1. Register new user\n"
              << "2. Login\n"
              << "3. Change password\n"
              << "4. Transfer points\n"
              << "5. View transaction history\n"
              << "6. Exit\n"
              << "Enter your choice: ";
}

void registerUser(UserManager& userManager) {
    std::string username, password, email;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;
    std::cout << "Enter email: ";
    std::cin >> email;
    
    if (userManager.registerUser(username, password, email)) {
        std::cout << "Registration successful!\n";
    } else {
        std::cout << "Registration failed. Username might already exist.\n";
    }
}

void login(UserManager& userManager, OTPManager& otpManager) {
    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;
    
    auto user = userManager.authenticateUser(username, password);
    if (user) {
        std::cout << "Login successful!\n";
        
        // Generate OTP for sensitive operations
        std::string otp = otpManager.generateAndStoreOTP(username);
        std::cout << "Your OTP for sensitive operations: " << otp << "\n";
    } else {
        std::cout << "Login failed. Invalid credentials.\n";
    }
}

void changePassword(UserManager& userManager, OTPManager& otpManager) {
    std::string username, oldPassword, newPassword, otp;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter old password: ";
    std::cin >> oldPassword;
    std::cout << "Enter new password: ";
    std::cin >> newPassword;
    std::cout << "Enter OTP: ";
    std::cin >> otp;
    
    if (otpManager.verifyOTP(username, otp)) {
        if (userManager.changeUserPassword(username, oldPassword, newPassword)) {
            std::cout << "Password changed successfully!\n";
        } else {
            std::cout << "Password change failed. Invalid credentials.\n";
        }
    } else {
        std::cout << "Invalid OTP.\n";
    }
}

void transferPoints(UserManager& userManager, OTPManager& otpManager) {
    std::string fromUsername, toUsername, otp;
    double amount;
    
    std::cout << "Enter your username: ";
    std::cin >> fromUsername;
    std::cout << "Enter recipient username: ";
    std::cin >> toUsername;
    std::cout << "Enter amount to transfer: ";
    std::cin >> amount;
    std::cout << "Enter OTP: ";
    std::cin >> otp;
    
    if (otpManager.verifyOTP(fromUsername, otp)) {
        // Get both users
        auto fromUser = userManager.authenticateUser(fromUsername, "");
        auto toUser = userManager.authenticateUser(toUsername, "");
        
        if (!fromUser || !toUser) {
            std::cout << "One or both users not found.\n";
            return;
        }
        
        // Create wallet objects
        Wallet fromWallet(fromUser->getWalletId());
        Wallet toWallet(toUser->getWalletId());
        
        // Perform transfer
        if (fromWallet.transfer(toWallet, amount)) {
            // Update user balances
            fromUser->updateBalance(-amount);
            toUser->updateBalance(amount);
            userManager.saveUsers();
            std::cout << "Transfer successful!\n";
        } else {
            std::cout << "Transfer failed. Insufficient balance or invalid amount.\n";
        }
    } else {
        std::cout << "Invalid OTP.\n";
    }
}

void viewTransactionHistory(UserManager& userManager) {
    std::string username;
    std::cout << "Enter username: ";
    std::cin >> username;
    
    auto user = userManager.authenticateUser(username, "");
    if (!user) {
        std::cout << "User not found.\n";
        return;
    }
    
    Wallet wallet(user->getWalletId());
    const auto& transactions = wallet.getTransactionHistory();
    
    if (transactions.empty()) {
        std::cout << "No transactions found.\n";
        return;
    }
    
    std::cout << "\nTransaction History for " << username << ":\n";
    std::cout << "----------------------------------------\n";
    for (const auto& t : transactions) {
        std::cout << "From: " << t.fromWalletId << "\n"
                  << "To: " << t.toWalletId << "\n"
                  << "Amount: " << t.amount << "\n"
                  << "Status: " << (t.completed ? "Completed" : "Failed") << "\n"
                  << "----------------------------------------\n";
    }
}

int main() {
    UserManager& userManager = *UserManager::getInstance();
    OTPManager& otpManager = *OTPManager::getInstance();
    
    int choice;
    do {
        printMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                registerUser(userManager);
                break;
            case 2:
                login(userManager, otpManager);
                break;
            case 3:
                changePassword(userManager, otpManager);
                break;
            case 4:
                transferPoints(userManager, otpManager);
                break;
            case 5:
                viewTransactionHistory(userManager);
                break;
            case 6:
                std::cout << "Goodbye!\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);
    
    return 0;
} 