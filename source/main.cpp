// source/main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <limits>   // For std::numeric_limits
#include <iomanip>  // For std::fixed, std::setprecision

// Config - Should be one of the first to be included by main
#include "Config.h" // Assuming it's directly in include/

// Models
#include "models/User.hpp"
#include "models/Wallet.hpp"
#include "models/Transaction.hpp"

// Utils
#include "utils/FileHandler.hpp"
#include "utils/HashUtils.hpp"
#include "utils/InputValidator.hpp"
#include "utils/Logger.hpp" // Logger.hpp itself doesn't include Config.h directly
#include "utils/TimeUtils.hpp"

// Services
#include "services/OTPService.hpp"
#include "services/AuthService.hpp"
#include "services/UserService.hpp"
#include "services/WalletService.hpp"
#include "services/AdminService.hpp"

// Global data stores (for simplicity in this console application)
std::vector<User> g_users;
std::vector<Wallet> g_wallets;
std::vector<Transaction> g_transactions;
std::optional<User> g_currentUser; // Currently logged-in user

// Forward declarations for menu handler functions
void displayMainMenu();
void displayUserMenu(const User& user);
void displayAdminMenu(const User& admin);

void handleRegistration(AuthService& authService, WalletService& walletService);
void handleLogin(AuthService& authService, UserService& userService /*needed for post-login update*/);
void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService);
void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService);

// Utility input functions
std::string getStringInput(const std::string& prompt, bool allowEmpty = false);
int getIntInput(const std::string& prompt);
double getDoubleInput(const std::string& prompt);
void clearScreen();
void pauseScreen(const std::string& message = "\nPress Enter to continue...");

// Add this new helper function after the other input functions
std::string getPasswordInput(const std::string& prompt, bool allowEmpty = false) {
    const int MAX_ATTEMPTS = 3;
    int attempts = 0;
    
    while (attempts < MAX_ATTEMPTS) {
        std::string input = getStringInput(prompt, allowEmpty);
        if (input.empty() && !allowEmpty) {
            std::cout << "Input cannot be empty. Please try again." << std::endl;
            attempts++;
            if (attempts >= MAX_ATTEMPTS) {
                std::cout << "Too many failed attempts. Returning to previous menu." << std::endl;
                pauseScreen();
                return ""; // Return empty string to indicate failure
            }
            continue;
        }
        return input;
    }
    return ""; // Return empty string if all attempts failed
}

int main() {
    // 1. Initialize Logger using AppConfig defaults
    // This static instance will be configured once.
    Logger::getInstance(
        std::string(AppConfig::LOG_DIRECTORY) + AppConfig::LOG_FILENAME,
        AppConfig::DEFAULT_CONSOLE_LOG_LEVEL,
        AppConfig::DEFAULT_FILE_LOG_LEVEL,
        AppConfig::DEFAULT_CONSOLE_LOGGING_ENABLED
    );
    LOG_INFO("===================================");
    LOG_INFO("Application Starting...");
    LOG_INFO("Version: " + std::string(AppConfig::APPLICATION_VERSION));
    LOG_INFO("Data Directory: " + std::string(AppConfig::DATA_DIRECTORY));
    LOG_INFO("===================================");


    // 2. Initialize Utilities and Services
    HashUtils hashUtils; // Constructor prints security warning for demo hash
    FileHandler fileHandler; // Uses AppConfig::DATA_DIRECTORY by default
    OTPService otpService;

    // 3. Load initial data
    LOG_INFO("Loading data...");
    if (!fileHandler.loadUsers(g_users)) {
        LOG_ERROR("Failed to load user data. Application might behave unexpectedly.");
    }
    if (!fileHandler.loadWallets(g_wallets)) {
        LOG_ERROR("Failed to load wallet data. Application might behave unexpectedly.");
    }
    if (!fileHandler.loadTransactions(g_transactions)) {
        LOG_ERROR("Failed to load transaction data. Application might behave unexpectedly.");
    }
    LOG_INFO("Data loading complete. Users: " + std::to_string(g_users.size()) +
             ", Wallets: " + std::to_string(g_wallets.size()) +
             ", Transactions: " + std::to_string(g_transactions.size()));

    AuthService authService(g_users, fileHandler, otpService, hashUtils);
    UserService userService(g_users, fileHandler, otpService);
    WalletService walletService(g_users, g_wallets, g_transactions, fileHandler, otpService, hashUtils);
    AdminService adminService(g_users, authService, userService, walletService);

    // --- Create a default Admin account if no users exist ---
    if (g_users.empty()) {
        LOG_INFO("No users found. Creating default Admin account...");
        std::string adminMsg, tempPass;
        if (adminService.adminCreateUserAccount("admin", "Administrator", "admin@reward.system", "000000000", UserRole::AdminUser, tempPass, adminMsg)) {
            LOG_INFO("Default Admin account created. Username: admin, Temp Password: " + tempPass);
            LOG_INFO("Message: " + adminMsg); // Will include wallet creation status
            LOG_INFO("Please change the password after first login.");
        } else {
            LOG_ERROR("Failed to create default Admin account: " + adminMsg);
        }
    }
    // --- End default Admin account creation ---

    bool running = true;
    while (running) {
        if (!g_currentUser.has_value()) { // Not logged in
            displayMainMenu();
            int choice = getIntInput("Your choice: ");
            switch (choice) {
                case 1:
                    handleRegistration(authService, walletService);
                    break;
                case 2:
                    handleLogin(authService, userService); // Pass userService to update g_currentUser
                    break;
                case 0:
                    running = false;
                    break;
                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    pauseScreen();
                    break;
            }
        } else { // Logged in
            // Ensure g_currentUser reflects the latest state from g_users
            // This is important if other operations modified the user in g_users
            bool userStillExists = false;
            for(const auto& u_db : g_users){
                if(u_db.userId == g_currentUser.value().userId){
                    g_currentUser = u_db; // Refresh current user data
                    userStillExists = true;
                    break;
                }
            }
            if (!userStillExists || g_currentUser.value().status == AccountStatus::Inactive) {
                LOG_WARNING("Current user session invalid (user deleted or inactive). Logging out.");
                std::cout << "Your session is no longer valid. You have been logged out." << std::endl;
                g_currentUser.reset();
                pauseScreen();
                continue; // Go back to main menu
            }


            if (g_currentUser.value().isTemporaryPassword) {
                clearScreen();
                std::cout << "--- TEMPORARY PASSWORD ---" << std::endl;
                std::cout << "You are using a temporary password. You must change it now." << std::endl;
                std::string newPass = getPasswordInput("Enter new password: ");
                if (newPass.empty()) {
                    g_currentUser.reset(); // Log out if password input failed
                    continue;
                }
                while (!InputValidator::isValidPassword(newPass)) {
                    std::cout << "Password is not strong enough (min "
                              << AppConfig::MIN_PASSWORD_LENGTH
                              << " chars, needs upper, lower, digit). Try again." << std::endl;
                    newPass = getPasswordInput("Enter new password: ");
                    if (newPass.empty()) {
                        g_currentUser.reset(); // Log out if password input failed
                        continue;
                    }
                }

                std::string confirmPass = getPasswordInput("Confirm new password: ");
                if (confirmPass.empty()) {
                    g_currentUser.reset(); // Log out if password input failed
                    continue;
                }

                if (newPass == confirmPass) {
                    User modifiableCurrentUser = g_currentUser.value();
                    std::string changeMsg;
                    if (authService.forceTemporaryPasswordChange(modifiableCurrentUser, newPass, changeMsg)) {
                        std::cout << changeMsg << std::endl;
                        g_currentUser = modifiableCurrentUser;
                        LOG_INFO("User '" + g_currentUser.value().username + "' changed temporary password.");
                    } else {
                        std::cout << "Error: " << changeMsg << std::endl;
                        LOG_ERROR("Failed temp password change for '" + g_currentUser.value().username + "': " + changeMsg);
                        std::cout << "Logging out for security." << std::endl;
                        g_currentUser.reset();
                    }
                } else {
                    std::cout << "Passwords do not match. Logging out for security." << std::endl;
                    LOG_WARNING("Temp password change passwords mismatch for '" + g_currentUser.value().username + "'. Logging out.");
                    g_currentUser.reset();
                }
                pauseScreen();
                continue;
            }
            else if (g_currentUser.value().role == UserRole::AdminUser) {
                handleAdminActions(adminService, userService, authService, walletService, otpService);
            } else {
                handleUserActions(userService, authService, walletService, otpService);
            }
        }
    }

    LOG_INFO("Application shutting down gracefully.");
    std::cout << "Thank you for using the Reward System!" << std::endl;
    return 0;
}

// --- Implementation of Helper and Handler Functions ---

void clearScreen() {
    // ANSI escape code for clearing screen (works on most modern terminals)
    std::cout << "\033[2J\033[1;1H";
    // On Windows, you might prefer: system("cls");
    // On Linux/macOS: system("clear");
    // (system calls are generally less portable and have security implications)
}

void pauseScreen(const std::string& message) {
    std::cout << message;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear previous newline
    std::string dummy;
    std::getline(std::cin, dummy); // Wait for user to press Enter
}

std::string getStringInput(const std::string& prompt, bool allowEmpty) {
    std::string input;
    while (true) {
        std::cout << prompt;
        // std::ws consumes leading whitespace before getline reads.
        // However, if cin buffer is dirty from previous unconsumed newlines,
        // it might still cause issues. The pauseScreen often helps clear this.
        // A more robust way is to always use getline and then parse.
        if (std::cin.peek() == '\n') { // Consume leftover newline from previous numeric input
            std::cin.ignore();
        }
        std::getline(std::cin, input);

        if (allowEmpty || InputValidator::isNonEmpty(input)) {
            return input;
        }
        std::cout << "Input cannot be empty. Please try again." << std::endl;
    }
}

int getIntInput(const std::string& prompt) {
    std::string inputStr;
    int value;
    while (true) {
        inputStr = getStringInput(prompt);
        if (InputValidator::isValidInteger(inputStr, value)) {
            return value;
        }
        std::cout << "Invalid integer input. Please try again." << std::endl;
    }
}

double getDoubleInput(const std::string& prompt) {
    std::string inputStr;
    double value;
    while (true) {
        inputStr = getStringInput(prompt);
        if (InputValidator::isValidDouble(inputStr, value)) {
            return value;
        }
        std::cout << "Invalid decimal number input. Please try again." << std::endl;
    }
}

void displayMainMenu() {
    clearScreen();
    std::cout << "===== " << AppConfig::APPLICATION_NAME << " v" << AppConfig::APPLICATION_VERSION << " =====" << std::endl;
    std::cout << "1. Register Account" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "0. Exit Application" << std::endl;
    std::cout << "==========================================" << std::endl;
}

void handleRegistration(AuthService& authService, WalletService& walletService) {
    clearScreen();
    std::cout << "--- Register New Account ---" << std::endl;
    std::string username, password, fullName, email, phone, msg;

    do { username = getStringInput("Enter username (3-20 chars, alphanumeric, '_'): "); } 
    while (!InputValidator::isValidUsername(username) && (std::cout << "Invalid username format. Try again.\n", true));
    
    password = getPasswordInput("Enter password (min " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " chars, upper, lower, digit): ");
    if (password.empty()) {
        return; // Return to main menu if password input failed
    }
    while (!InputValidator::isValidPassword(password)) {
        std::cout << "Password not strong enough. Try again." << std::endl;
        password = getPasswordInput("Enter password (min " + std::to_string(AppConfig::MIN_PASSWORD_LENGTH) + " chars, upper, lower, digit): ");
        if (password.empty()) {
            return; // Return to main menu if password input failed
        }
    }

    fullName = getStringInput("Enter full name: ");
    do { email = getStringInput("Enter email: "); } 
    while (!InputValidator::isValidEmail(email) && (std::cout << "Invalid email format. Try again.\n", true));
    do { phone = getStringInput("Enter phone number: "); } 
    while (!InputValidator::isValidPhoneNumber(phone) && (std::cout << "Invalid phone number format. Try again.\n", true));

    if (authService.registerUser(username, password, fullName, email, phone, UserRole::RegularUser, msg)) {
        std::cout << "Registration: " << msg << std::endl;
        // Auto-create wallet for the new user
        User newUser; // Find the newly created user to get their ID
        for(const auto& u : g_users){ if(u.username == username){ newUser = u; break; } }

        if(!newUser.userId.empty()){
            std::string walletMsg;
            if(walletService.createWalletForUser(newUser.userId, walletMsg)){
                std::cout << "Wallet creation: " << walletMsg << std::endl;
            } else {
                LOG_ERROR("Wallet creation failed for user '" + newUser.username + "' after registration: " + walletMsg);
                std::cout << "Error creating wallet: " << walletMsg << std::endl;
            }
        } else {
            LOG_ERROR("Could not find user '" + username + "' after registration to create wallet.");
        }
    } else {
        std::cout << "Registration failed: " << msg << std::endl;
    }
    pauseScreen();
}

void handleLogin(AuthService& authService, UserService& userService) {
    clearScreen();
    std::cout << "--- Login ---" << std::endl;
    std::string username = getStringInput("Username: ");
    std::string msg;
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;

    while (attempts < MAX_ATTEMPTS) {
        std::string password = getPasswordInput("Password: ");
        if (password.empty()) {
            return; // Return to main menu if password input failed
        }

        std::optional<User> userOpt = authService.loginUser(username, password, msg);
        if (userOpt) {
            g_currentUser = userOpt.value();
            std::cout << msg << " Welcome, " << g_currentUser.value().fullName << "!" << std::endl;
            LOG_INFO("User '" + g_currentUser.value().username + "' logged in.");
            pauseScreen();
            return;
        } else {
            std::cout << "Login failed: " << msg << std::endl;
            attempts++;
            if (attempts >= MAX_ATTEMPTS) {
                std::cout << "Too many failed attempts. Returning to main menu." << std::endl;
                pauseScreen();
                return;
            }
        }
    }
}

void displayUserMenu(const User& user) {
    clearScreen();
    std::cout << "===== USER MENU (" << user.username << ") =====" << std::endl;
    std::cout << "1. View Profile" << std::endl;
    std::cout << "2. Update Profile" << std::endl;
    std::cout << "3. Change Password" << std::endl;
    std::cout << "4. Setup/View OTP" << std::endl;
    std::cout << "5. View Wallet Balance" << std::endl;
    std::cout << "6. Transfer Points" << std::endl;
    std::cout << "7. View Transaction History" << std::endl;
    std::cout << "9. Logout" << std::endl;
    std::cout << "0. Exit Application" << std::endl;
    std::cout << "==============================" << std::endl;
}

void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService) {
    // g_currentUser is guaranteed to have a value here
    User& CUser = g_currentUser.value(); // Get a reference to the global current user for direct updates if needed

    displayUserMenu(CUser);
    int choice = getIntInput("Your choice: ");
    std::string msg, otpInput;

    switch (choice) {
        case 1: // View Profile
            clearScreen();
            std::cout << "--- Your Profile ---" << std::endl;
            std::cout << "User ID: " << CUser.userId << std::endl;
            std::cout << "Username: " << CUser.username << std::endl;
            std::cout << "Full Name: " << CUser.fullName << std::endl;
            std::cout << "Email: " << CUser.email << std::endl;
            std::cout << "Phone: " << CUser.phoneNumber << std::endl;
            std::cout << "Role: " << User::roleToString(CUser.role) << std::endl;
            std::cout << "Status: " << User::statusToString(CUser.status) << std::endl;
            std::cout << "OTP Enabled: " << (CUser.otpSecretKey.empty() ? "No" : "Yes") << std::endl;
            pauseScreen();
            break;

        case 2: { // Update Profile
            clearScreen();
            std::cout << "--- Update Profile ---" << std::endl;
            std::string newFullName = getStringInput("New Full Name (current: " + CUser.fullName + ", leave empty to keep): ", true);
            std::string newEmail;
            do { newEmail = getStringInput("New Email (current: " + CUser.email + ", leave empty to keep): ", true);
            } while (!newEmail.empty() && !InputValidator::isValidEmail(newEmail) && (std::cout << "Invalid email format.\n", true));
            std::string newPhone;
            do { newPhone = getStringInput("New Phone (current: " + CUser.phoneNumber + ", leave empty to keep): ", true);
            } while (!newPhone.empty() && !InputValidator::isValidPhoneNumber(newPhone) && (std::cout << "Invalid phone format.\n", true));

            if (newFullName.empty()) newFullName = CUser.fullName;
            if (newEmail.empty()) newEmail = CUser.email;
            if (newPhone.empty()) newPhone = CUser.phoneNumber;
            
            otpInput = "";
            if (!CUser.otpSecretKey.empty()) {
                otpInput = getStringInput("Enter your OTP code to confirm changes: ");
            }

            if (userService.updateUserProfile(CUser.userId, newFullName, newEmail, newPhone, otpInput, msg)) {
                std::cout << "Success: " << msg << std::endl;
                // Refresh g_currentUser from g_users which userService modified
                auto updatedUserOpt = userService.getUserProfile(CUser.userId);
                if (updatedUserOpt) g_currentUser = updatedUserOpt.value();
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 3: { // Change Password
            clearScreen();
            std::cout << "--- Change Password ---" << std::endl;
            std::string oldPass = getPasswordInput("Enter current password: ");
            if (oldPass.empty()) {
                return; // Return to user menu if password input failed
            }

            std::string newPass = getPasswordInput("Enter new password: ");
            if (newPass.empty()) {
                return; // Return to user menu if password input failed
            }
            while (!InputValidator::isValidPassword(newPass)) {
                std::cout << "Password not strong enough." << std::endl;
                newPass = getPasswordInput("Enter new password: ");
                if (newPass.empty()) {
                    return; // Return to user menu if password input failed
                }
            }

            std::string confirmPass = getPasswordInput("Confirm new password: ");
            if (confirmPass.empty()) {
                return; // Return to user menu if password input failed
            }

            if (newPass == confirmPass) {
                if (authService.changePassword(CUser.userId, oldPass, newPass, msg)) {
                    std::cout << "Success: " << msg << std::endl;
                    auto updatedUserOpt = userService.getUserProfile(CUser.userId);
                    if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
                } else {
                    std::cout << "Failed: " << msg << std::endl;
                }
            } else {
                std::cout << "New passwords do not match." << std::endl;
            }
            pauseScreen();
            break;
        }
        case 4: { // Setup/View OTP
            clearScreen();
            std::cout << "--- OTP Setup ---" << std::endl;
            if (CUser.otpSecretKey.empty()) {
                std::cout << "OTP is not yet enabled." << std::endl;
                std::string setupChoice = getStringInput("Do you want to set up OTP now? (y/n): ");
                if (setupChoice == "y" || setupChoice == "Y") {
                    std::optional<std::string> secretOpt = authService.setupOtpForUser(CUser.userId, msg);
                    if (secretOpt) {
                        std::cout << "Success: " << msg << std::endl;
                        std::cout << "Your OTP Secret Key (Base32): " << secretOpt.value() << std::endl;
                        std::cout << "Scan this URI with your Authenticator app (or add manually):" << std::endl;
                        std::cout << otpService.generateOtpUri(CUser.username, secretOpt.value()) << std::endl;
                        // Refresh g_currentUser
                        auto updatedUserOpt = userService.getUserProfile(CUser.userId);
                        if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
                    } else {
                        std::cout << "Failed: " << msg << std::endl;
                    }
                }
            } else {
                std::cout << "OTP is already enabled for your account." << std::endl;
                std::cout << "For security, your secret key is not displayed again." << std::endl;
                std::cout << "If you need to re-setup, a 'Disable OTP' feature would be required first (not implemented)." << std::endl;
                 std::cout << "You can use this URI to re-add to an authenticator if you have your secret:" << std::endl;
                 std::cout << otpService.generateOtpUri(CUser.username, CUser.otpSecretKey) << std::endl;
            }
            pauseScreen();
            break;
        }
        case 5: { // View Wallet Balance
            clearScreen();
            std::cout << "--- Wallet Balance ---" << std::endl;
            auto walletOpt = walletService.getWalletByUserId(CUser.userId);
            if (walletOpt) {
                std::cout << "Current Balance: " << std::fixed << std::setprecision(2) 
                          << walletOpt.value().balance << " " << AppConfig::APPLICATION_NAME << " Points" << std::endl;
            } else {
                std::cout << "Could not retrieve wallet information. Please contact support." << std::endl;
                 LOG_ERROR("Wallet not found for logged in user: " + CUser.username);
            }
            pauseScreen();
            break;
        }
        case 6: { // Transfer Points
            clearScreen();
            std::cout << "--- Transfer Points ---" << std::endl;
            auto senderWalletOpt = walletService.getWalletByUserId(CUser.userId);
            if (!senderWalletOpt) {
                std::cout << "Error: Your wallet information could not be found." << std::endl;
                pauseScreen();
                break;
            }
            std::string receiverWalletId = getStringInput("Enter Receiver's Wallet ID: ");
            double amount = getDoubleInput("Enter amount to transfer: ");
            
            otpInput = "";
            if (!CUser.otpSecretKey.empty()) {
                otpInput = getStringInput("Enter your OTP code: ");
            }

            if (walletService.transferPoints(CUser.userId, senderWalletOpt.value().walletId, receiverWalletId, amount, otpInput, msg)) {
                std::cout << "Success: " << msg << std::endl;
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 7: { // View Transaction History
            clearScreen();
            std::cout << "--- Transaction History ---" << std::endl;
            auto walletOpt = walletService.getWalletByUserId(CUser.userId);
            if (walletOpt) {
                std::vector<Transaction> history = walletService.getTransactionHistory(walletOpt.value().walletId);
                if (history.empty()) {
                    std::cout << "No transactions found." << std::endl;
                } else {
                    for (const auto& tx : history) {
                        std::cout << "----------------------------------" << std::endl;
                        std::cout << "Tx ID: " << tx.transactionId << std::endl;
                        std::cout << "Date: " << TimeUtils::formatTimestamp(tx.transactionTimestamp) << std::endl;
                        std::cout << "Type: " << (tx.senderWalletId == walletOpt.value().walletId ? "Sent" : "Received") << std::endl;
                        std::cout << "From: " << tx.senderWalletId << std::endl;
                        std::cout << "To: " << tx.receiverWalletId << std::endl;
                        std::cout << "Amount: " << std::fixed << std::setprecision(2) << tx.amountTransferred << std::endl;
                        std::cout << "Status: " << Transaction::statusToString(tx.status) << std::endl;
                        if (!tx.description.empty()) {
                            std::cout << "Description: " << tx.description << std::endl;
                        }
                    }
                    std::cout << "----------------------------------" << std::endl;
                }
            } else {
                std::cout << "Could not retrieve wallet information." << std::endl;
            }
            pauseScreen();
            break;
        }
        case 9: // Logout
            LOG_INFO("User '" + CUser.username + "' logged out.");
            g_currentUser.reset(); // Clear current user
            std::cout << "You have been logged out." << std::endl;
            pauseScreen();
            // The main loop will detect no currentUser and show main menu
            return; // Exit user actions and let main loop continue
        case 0: // Exit Application
            LOG_INFO("User '" + CUser.username + "' chose to exit application.");
            g_currentUser.reset(); // Log out before exiting
            // To exit the main loop in main():
            // We need a way for this function to signal main loop to stop.
            // For now, direct exit. A better way is to return a bool from handleUserActions.
            std::cout << "Exiting application..." << std::endl;
            exit(0); // Exit the entire program
            break; 
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            pauseScreen();
            break;
    }
}

void displayAdminMenu(const User& admin) {
    clearScreen();
    std::cout << "===== ADMIN MENU (" << admin.username << ") =====" << std::endl;
    std::cout << "--- Self Management ---" << std::endl;
    std::cout << "1. View My Admin Profile" << std::endl;
    std::cout << "2. Update My Admin Profile" << std::endl;
    std::cout << "3. Change My Admin Password" << std::endl;
    std::cout << "4. Setup/View My Admin OTP" << std::endl;
    std::cout << "--- User Management ---" << std::endl;
    std::cout << "11. List All Users" << std::endl;
    std::cout << "12. Create New User Account" << std::endl;
    std::cout << "13. Update User Profile" << std::endl;
    std::cout << "14. Activate User Account" << std::endl;
    std::cout << "15. Deactivate User Account" << std::endl;
    std::cout << "--- Wallet Management ---" << std::endl;
    std::cout << "21. Deposit Points to User Wallet" << std::endl;
    std::cout << "--- System ---" << std::endl;
    std::cout << "9. Logout" << std::endl;
    std::cout << "0. Exit Application" << std::endl;
    std::cout << "==============================" << std::endl;
}

void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService) {
    // g_currentUser is guaranteed to be an admin here
    User& CAdmin = g_currentUser.value();

    displayAdminMenu(CAdmin);
    int choice = getIntInput("Your choice: ");
    std::string msg, otpInput; // For admin's own OTP if needed for an action

    switch (choice) {
        // Admin's self-management actions (can reuse parts of handleUserActions or have dedicated logic)
        case 1: case 2: case 3: case 4:
            LOG_DEBUG("Admin accessing self-management. Redirecting to user actions for admin's own account.");
            // For simplicity, we treat the admin as a user for these actions.
            // A more granular system might have separate handlers or checks.
            handleUserActions(userService, authService, walletService, otpService);
            // After handleUserActions, g_currentUser might have changed (e.g., logout).
            // The main loop will re-evaluate.
            return; // Return to main loop to re-evaluate state

        // User Management
        case 11: { // List All Users
            clearScreen();
            std::cout << "--- All Users ---" << std::endl;
            std::vector<User> allUsers = adminService.listAllUsers();
            if (allUsers.empty()) {
                std::cout << "No users in the system." << std::endl;
            } else {
                for (const auto& u : allUsers) {
                    std::cout << "ID: " << u.userId << "\n Username: " << u.username
                              << "\n Name: " << u.fullName << "\n Email: " << u.email
                              << "\n Phone: " << u.phoneNumber
                              << "\n Role: " << User::roleToString(u.role)
                              << "\n Status: " << User::statusToString(u.status) 
                              << "\n OTP Set: " << (u.otpSecretKey.empty() ? "No" : "Yes")
                              << "\n--------------------------" << std::endl;
                }
            }
            pauseScreen();
            break;
        }
        case 12: { // Create New User Account
            clearScreen();
            std::cout << "--- Admin: Create New User Account ---" << std::endl;
            std::string newUsername, newPassword, newFullName, newEmail, newPhone, tempPass;
            UserRole newUserRole = UserRole::RegularUser; // Default to regular user

            do { newUsername = getStringInput("Enter username for new user: "); } while (!InputValidator::isValidUsername(newUsername) && (std::cout << "Invalid username format.\n", true));
            newFullName = getStringInput("Enter full name for new user: ");
            do { newEmail = getStringInput("Enter email for new user: "); } while (!InputValidator::isValidEmail(newEmail) && (std::cout << "Invalid email format.\n", true));
            do { newPhone = getStringInput("Enter phone for new user: "); } while (!InputValidator::isValidPhoneNumber(newPhone) && (std::cout << "Invalid phone format.\n", true));
            
            // Admin cannot create another admin via this simplified interface for now.
            if (adminService.adminCreateUserAccount(newUsername, newFullName, newEmail, newPhone, newUserRole, tempPass, msg)) {
                std::cout << "\n=== IMPORTANT: USER ACCOUNT CREATED SUCCESSFULLY ===\n" << std::endl;
                std::cout << "Account Details:" << std::endl;
                std::cout << "Username: " << newUsername << std::endl;
                std::cout << "Full Name: " << newFullName << std::endl;
                std::cout << "Email: " << newEmail << std::endl;
                std::cout << "Phone: " << newPhone << std::endl;
                std::cout << "\n=== TEMPORARY PASSWORD ===\n" << std::endl;
                std::cout << "The user's temporary password is: " << tempPass << std::endl;
                std::cout << "\nIMPORTANT: Please provide this temporary password to the user securely." << std::endl;
                std::cout << "The user will be required to change this password upon first login." << std::endl;
                std::cout << "\n==========================================\n" << std::endl;
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 13: { // Update User Profile
            clearScreen();
            std::cout << "--- Admin: Update User Profile ---" << std::endl;
            std::string targetUserId = getStringInput("Enter User ID of the user to update: ");
            auto targetUserOpt = userService.getUserProfile(targetUserId);
            if (!targetUserOpt) {
                std::cout << "User with ID '" << targetUserId << "' not found." << std::endl;
                pauseScreen();
                break;
            }
            User targetUser = targetUserOpt.value(); // Get a copy for display and checks
            std::cout << "Updating profile for: " << targetUser.username << " (" << targetUser.fullName << ")" << std::endl;

            std::string newFullName = getStringInput("New Full Name (current: " + targetUser.fullName + ", leave empty to keep): ", true);
            std::string newEmail;
            do { newEmail = getStringInput("New Email (current: " + targetUser.email + ", leave empty to keep): ", true);
            } while (!newEmail.empty() && !InputValidator::isValidEmail(newEmail) && (std::cout << "Invalid email format.\n", true));
            std::string newPhone;
            do { newPhone = getStringInput("New Phone (current: " + targetUser.phoneNumber + ", leave empty to keep): ", true);
            } while (!newPhone.empty() && !InputValidator::isValidPhoneNumber(newPhone) && (std::cout << "Invalid phone format.\n", true));
            
            std::cout << "Current Status: " << User::statusToString(targetUser.status) << std::endl;
            std::cout << "New Status (0=NotActivated, 1=Active, 2=Inactive, Enter to keep current): ";
            std::string statusStr = getStringInput("", true);
            AccountStatus newStatus = targetUser.status;
            if(!statusStr.empty()){
                int statusInt = -1;
                if(InputValidator::isValidInteger(statusStr, statusInt)){
                    if(statusInt == 0) newStatus = AccountStatus::NotActivated;
                    else if (statusInt == 1) newStatus = AccountStatus::Active;
                    else if (statusInt == 2) newStatus = AccountStatus::Inactive;
                    else std::cout << "Invalid status choice. Status unchanged.\n";
                } else std::cout << "Invalid status input. Status unchanged.\n";
            }

            if (newFullName.empty()) newFullName = targetUser.fullName;
            if (newEmail.empty()) newEmail = targetUser.email;
            if (newPhone.empty()) newPhone = targetUser.phoneNumber;

            std::string targetUserOtp = "";
            if (!targetUser.otpSecretKey.empty()) {
                targetUserOtp = getStringInput("Enter TARGET USER's OTP code (if they provided it): ", true);
            }

            if (adminService.adminUpdateUserProfile(CAdmin.userId, targetUserId, newFullName, newEmail, newPhone, newStatus, targetUserOtp, msg)) {
                std::cout << "Success: " << msg << std::endl;
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 14: // Activate User
            clearScreen();
            std::cout << "--- Admin: Activate User Account ---" << std::endl;
            if(adminService.adminActivateUser(getStringInput("Enter User ID to activate: "), msg)){
                 std::cout << "Success: " << msg << std::endl;
            } else {
                 std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        case 15: // Deactivate User
            clearScreen();
            std::cout << "--- Admin: Deactivate User Account ---" << std::endl;
            if(adminService.adminDeactivateUser(getStringInput("Enter User ID to deactivate: "), msg)){
                std::cout << "Success: " << msg << std::endl;
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;

        // Wallet Management
        case 21: { // Deposit Points
            clearScreen();
            std::cout << "--- Admin: Deposit Points ---" << std::endl;
            std::string targetUserId = getStringInput("Enter User ID to deposit points to: ");
            double amount = getDoubleInput("Enter amount to deposit: ");
            std::string reason = getStringInput("Reason for deposit: ");
            if (adminService.adminDepositToUserWallet(CAdmin.userId, targetUserId, amount, reason, msg)) {
                std::cout << "Success: " << msg << std::endl;
            } else {
                std::cout << "Failed: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }

        case 9: // Logout
            LOG_INFO("Admin '" + CAdmin.username + "' logged out.");
            g_currentUser.reset();
            std::cout << "You have been logged out." << std::endl;
            pauseScreen();
            return; 
        case 0: // Exit Application
            LOG_INFO("Admin '" + CAdmin.username + "' chose to exit application.");
            g_currentUser.reset();
            std::cout << "Exiting application..." << std::endl;
            exit(0);
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            pauseScreen();
            break;
    }
}