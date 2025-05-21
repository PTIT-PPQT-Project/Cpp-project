// source/main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <limits> 
#include <iomanip> 

// --- MOVE ALL INCLUDES HERE ---
// Models
#include "../include/models/User.hpp"
#include "../include/models/Wallet.hpp"
#include "../include/models/Transaction.hpp"

// Utils
#include "../include/utils/FileHandler.hpp"
#include "../include/utils/HashUtils.hpp"
#include "../include/utils/InputValidator.hpp"
#include "../include/utils/Logger.hpp"
#include "../include/utils/TimeUtils.hpp"
#include "../include/utils/DataInitializer.hpp"

// Services
#include "../include/services/OTPService.hpp"   // <<< ENSURE THESE ARE PRESENT AND CORRECT
#include "../include/services/AuthService.hpp"
#include "../include/services/UserService.hpp"
#include "../include/services/WalletService.hpp"
#include "../include/services/AdminService.hpp"
// --- END OF INCLUDES ---


// Global data or application state
std::vector<User> g_users;
std::vector<Wallet> g_wallets;
std::vector<Transaction> g_transactions;
std::optional<User> g_currentUser;

// Add a global flag for application exit
bool g_shouldExit = false;

// Forward declarations for handler functions (now the types should be known)
void displayMainMenu();
void displayUserMenu(const User& user); // User type is known from User.hpp
void displayAdminMenu(const User& admin); // User type is known

// Prototypes for handler functions - types like AuthService, WalletService should now be known
void handleRegistration(AuthService& authService, WalletService& walletService);
void handleLogin(AuthService& authService);
void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService, int choice);
void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService, int choice);

// Utility input functions
std::string getStringInput(const std::string& prompt, bool allowEmpty = false);
int getIntInput(const std::string& prompt);
double getDoubleInput(const std::string& prompt);
void clearScreen();
void pauseScreen();


int main() {
    // 1. Khởi tạo Logger
    Logger::getInstance("logs/app.log", LogLevel::INFO, LogLevel::DEBUG, false);
    LOG_INFO("Ung dung khoi dong.");

    // 2. Khởi tạo các Utilities và Services
    FileHandler fileHandler;
    HashUtils hashUtils;
    OTPService otpService;
    AuthService authService(g_users, fileHandler, otpService, hashUtils);
    UserService userService(g_users, fileHandler, otpService);
    WalletService walletService(g_users, g_wallets, g_transactions, fileHandler, otpService, hashUtils);
    AdminService adminService(g_users, authService, userService, walletService);

    // 3. Khởi tạo các file dữ liệu nếu chưa tồn tại
    LOG_INFO("Kiem tra va khoi tao du lieu...");
    if (!DataInitializer::initializeDataFiles()) {
        LOG_ERROR("Khong the khoi tao du lieu. Ung dung se ket thuc.");
        return 1;
    }

    // 4. Tải dữ liệu ban đầu
    LOG_INFO("Dang tai du lieu...");
    if (!fileHandler.loadUsers(g_users)) {
        LOG_ERROR("Khong the tai du lieu nguoi dung. Co the file bi loi hoac khong ton tai.");
    } else {
        LOG_INFO("Tai " + std::to_string(g_users.size()) + " nguoi dung thanh cong.");
    }
    if (!fileHandler.loadWallets(g_wallets)) {
        LOG_ERROR("Khong the tai du lieu vi. Co the file bi loi hoac khong ton tai.");
    } else {
        LOG_INFO("Tai " + std::to_string(g_wallets.size()) + " vi thanh cong.");
    }
    if (!fileHandler.loadTransactions(g_transactions)) {
        LOG_ERROR("Khong the tai du lieu giao dich. Co the file bi loi hoac khong ton tai.");
    } else {
        LOG_INFO("Tai " + std::to_string(g_transactions.size()) + " giao dich thanh cong.");
    }


    // ---- Tạo tài khoản Admin mẫu nếu chưa có ----
    bool adminExists = false;
    for(const auto& u : g_users) {
        if(u.role == UserRole::AdminUser) {
            adminExists = true;
            break;
        }
    }
    if (!adminExists && g_users.empty()) { // Chỉ tạo nếu chưa có admin và chưa có user nào
        LOG_INFO("Khong tim thay tai khoan Admin. Dang tao tai khoan Admin mac dinh...");
        std::string adminMsg;
        // Mật khẩu cho tài khoản admin mẫu này sẽ được in ra console khi tạo.
        // Trong thực tế, điều này cần được xử lý cẩn thận hơn.
        std::string tempPass = authService.createAccountWithTemporaryPassword(
            "admin", "Administrator", "admin@example.com", "0123456789", UserRole::AdminUser, adminMsg
        );
        if (!tempPass.empty()) {
            LOG_INFO("Tao tai khoan Admin thanh cong. Ten dang nhap: admin, Mat khau tam thoi: " + tempPass);
            LOG_INFO("Vui long doi mat khau sau khi dang nhap lan dau.");
            // Admin cũng cần ví
            for(const auto& u : g_users){
                if(u.username == "admin"){
                    std::string walletMsg;
                    walletService.createWalletForUser(u.userId, walletMsg);
                    LOG_INFO(walletMsg);
                    break;
                }
            }
        } else {
            LOG_ERROR("Tao tai khoan Admin mac dinh that bai: " + adminMsg);
        }
    }
    // ---- Kết thúc tạo tài khoản Admin mẫu ----


    bool running = true;
    while (running) {
        if (g_shouldExit) {
            running = false;
            continue;
        }
        
        if (!g_currentUser.has_value()) {
            displayMainMenu();
            int choice = getIntInput("Lua chon cua ban: ");
            switch (choice) {
                case 1: // Đăng ký
                    handleRegistration(authService, walletService);
                    break;
                case 2: // Đăng nhập
                    handleLogin(authService);
                    // Kiểm tra nếu đăng nhập thành công và là mật khẩu tạm
                    if (g_currentUser.has_value() && g_currentUser.value().isTemporaryPassword) {
                        std::cout << "Ban dang su dung mat khau tam thoi. Vui long doi mat khau moi." << std::endl;
                        std::string newPass = getStringInput("Nhap mat khau moi: ");
                        std::string confirmPass = getStringInput("Xac nhan mat khau moi: ");
                        if (newPass == confirmPass) {
                            std::string changeMsg;
                            // Cần lấy đối tượng User không phải const để authService có thể thay đổi
                            User modifiableUser = g_currentUser.value();
                            if (authService.forceTemporaryPasswordChange(modifiableUser, newPass, changeMsg)) {
                                std::cout << changeMsg << std::endl;
                                g_currentUser = modifiableUser; // Cập nhật lại currentUser với trạng thái mới
                            } else {
                                std::cout << "Loi: " << changeMsg << std::endl;
                                g_currentUser.reset(); // Đăng xuất nếu đổi mk tạm thất bại
                            }
                        } else {
                            std::cout << "Mat khau xac nhan khong khop. Vui long dang nhap lai de thu lai." << std::endl;
                            g_currentUser.reset(); // Đăng xuất
                        }
                         pauseScreen();
                    }
                    break;
                case 0: // Thoát
                    running = false;
                    break;
                default:
                    std::cout << "Lua chon khong hop le. Vui long chon lai." << std::endl;
                    pauseScreen();
                    break;
            }
        } else { // Đã đăng nhập
            User& currentUserRef = g_currentUser.value(); // Lấy tham chiếu để có thể cập nhật
            if (currentUserRef.role == UserRole::AdminUser) {
                LOG_INFO("Admin user " + currentUserRef.username + " accessing admin menu");
                displayAdminMenu(currentUserRef);
                int choice = getIntInput("Lua chon cua ban: ");
                handleAdminActions(adminService, userService, authService, walletService, otpService, choice);
            } else {
                LOG_INFO("Regular user " + currentUserRef.username + " accessing user menu");
                displayUserMenu(currentUserRef);
                int choice = getIntInput("Lua chon cua ban: ");
                handleUserActions(userService, authService, walletService, otpService, choice);
            }
        }
    }

    LOG_INFO("Ung dung ket thuc.");
    return 0;
}

// --- Implementations of helper functions ---

void clearScreen() {
    // Đơn giản cho đa nền tảng, có thể không hoàn hảo
    // Trên Windows: system("cls");
    // Trên Linux/macOS: system("clear");
    // Dùng ANSI escape code (hoạt động trên nhiều terminal hiện đại)
    std::cout << "\033[2J\033[1;1H";
    // Hoặc đơn giản là in nhiều dòng mới
    // for (int i = 0; i < 50; ++i) std::cout << std::endl;
}

void pauseScreen() {
    std::cout << "\nNhan Enter de tiep tuc...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Xóa buffer trước khi getline
    std::string dummy;
    std::getline(std::cin, dummy);
}


std::string getStringInput(const std::string& prompt, bool allowEmpty) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (allowEmpty || InputValidator::isNonEmpty(input)) {
            return input;
        }
        std::cout << "Dau vao khong duoc de trong. Vui long nhap lai." << std::endl;
    }
}

int getIntInput(const std::string& prompt) {
    std::string input;
    int value;
    while (true) {
        input = getStringInput(prompt);
        if (InputValidator::isValidInteger(input, value)) {
            return value;
        }
        std::cout << "Dau vao khong phai la so nguyen hop le. Vui long nhap lai." << std::endl;
    }
}

double getDoubleInput(const std::string& prompt) {
    std::string input;
    double value;
    while (true) {
        input = getStringInput(prompt);
        if (InputValidator::isValidDouble(input, value)) {
            return value;
        }
        std::cout << "Dau vao khong phai la so thuc hop le. Vui long nhap lai." << std::endl;
    }
}

void displayMainMenu() {
    clearScreen();
    std::cout << "===== HE THONG VI DIEM THUONG =====" << std::endl;
    std::cout << "1. Dang ky" << std::endl;
    std::cout << "2. Dang nhap" << std::endl;
    std::cout << "0. Thoat" << std::endl;
    std::cout << "===================================" << std::endl;
}

void handleRegistration(AuthService& authService, WalletService& walletService) {
    clearScreen();
    std::cout << "--- Dang Ky Tai Khoan ---" << std::endl;
    std::cout << "Nhan 'b' de quay lai menu chinh" << std::endl;
    std::string username, password, fullName, email, phone;
    
    while(true){
        username = getStringInput("Ten dang nhap (3-20 ky tu, chu cai, so, dau _): ");
        if(username == "b") {
            std::cout << "Quay lai menu chinh..." << std::endl;
            pauseScreen();
            return;
        }
        if(!InputValidator::isValidUsername(username)) {
            std::cout << "Ten dang nhap khong hop le.\n";
            continue;
        }
        
        if(authService.isUsernameExists(username)) {
            std::cout << "Ten dang nhap da ton tai. Vui long chon ten khac.\n";
            continue;
        }
        
        break;
    }
    while(true){
        password = getStringInput("Mat khau (min 8 ky tu, co chu hoa, thuong, so, ky tu dac biet): ");
        if(password == "b") {
            std::cout << "Quay lai menu chinh..." << std::endl;
            pauseScreen();
            return;
        }
        if(InputValidator::isValidPassword(password)) break;
        std::cout << "Mat khau khong du manh.\n";
    }
    fullName = getStringInput("Ho ten day du: ");
    if(fullName == "b") {
        std::cout << "Quay lai menu chinh..." << std::endl;
        pauseScreen();
        return;
    }
    while(true){
        email = getStringInput("Email: ");
        if(email == "b") {
            std::cout << "Quay lai menu chinh..." << std::endl;
            pauseScreen();
            return;
        }
        if(InputValidator::isValidEmail(email)) break;
        std::cout << "Email khong hop le.\n";
    }
    while(true){
        phone = getStringInput("So dien thoai: ");
        if(phone == "b") {
            std::cout << "Quay lai menu chinh..." << std::endl;
            pauseScreen();
            return;
        }
        if(InputValidator::isValidPhoneNumber(phone)) break;
        std::cout << "So dien thoai khong hop le.\n";
    }

    std::string msg;
    if (authService.registerUser(username, password, fullName, email, phone, UserRole::RegularUser, msg)) {
        std::cout << msg << std::endl;
        // Tự động tạo ví cho người dùng mới
        User newUser; // Cần tìm lại user vừa tạo để lấy ID
        for(const auto& u : g_users){
            if(u.username == username){
                newUser = u;
                break;
            }
        }
        if(!newUser.userId.empty()){
            std::string walletMsg;
            walletService.createWalletForUser(newUser.userId, walletMsg);
        }
    } else {
        std::cout << "Dang ky that bai: " << msg << std::endl;
    }
    pauseScreen();
}

void handleLogin(AuthService& authService) {
    clearScreen();
    std::cout << "--- Dang Nhap ---" << std::endl;
    std::cout << "Nhan 'b' de quay lai menu chinh" << std::endl;
    std::string username = getStringInput("Ten dang nhap: ");
    if(username == "b") {
        std::cout << "Quay lai menu chinh..." << std::endl;
        pauseScreen();
        return;
    }
    std::string password = getStringInput("Mat khau: ");
    if(password == "b") {
        std::cout << "Quay lai menu chinh..." << std::endl;
        pauseScreen();
        return;
    }
    std::string msg;
    std::optional<User> userOpt = authService.loginUser(username, password, msg);
    if (userOpt) {
        g_currentUser = userOpt.value();
        LOG_INFO("User " + username + " logged in with role: " + User::roleToString(g_currentUser.value().role));
        std::cout << msg << " Chao mung, " << g_currentUser.value().fullName << "!" << std::endl;
    } else {
        std::cout << "Dang nhap that bai: " << msg << std::endl;
    }
    pauseScreen();
}


void displayUserMenu(const User& user) {
    clearScreen();
    std::cout << "===== MENU NGUOI DUNG (" << user.username << ") =====" << std::endl;
    std::cout << "1. Xem thong tin ca nhan" << std::endl;
    std::cout << "2. Cap nhat thong tin ca nhan" << std::endl;
    std::cout << "3. Doi mat khau" << std::endl;
    std::cout << "4. Thiet lap/Xem OTP" << std::endl;
    std::cout << "5. Xem so du vi" << std::endl;
    std::cout << "6. Chuyen diem" << std::endl;
    std::cout << "7. Xem lich su giao dich" << std::endl;
    std::cout << "9. Dang xuat" << std::endl;
    std::cout << "0. Thoat ung dung" << std::endl;
    std::cout << "===================================" << std::endl;
}

void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService, int choice) {
    User& user = g_currentUser.value();
    std::string msg, otpCode;

    switch (choice) {
        case 1: { // Xem thông tin
            clearScreen();
            std::cout << "--- Thong Tin Ca Nhan ---" << std::endl;
            std::cout << "ID: " << user.userId << std::endl;
            std::cout << "Ten dang nhap: " << user.username << std::endl;
            std::cout << "Ho ten: " << user.fullName << std::endl;
            std::cout << "Email: " << user.email << std::endl;
            std::cout << "So dien thoai: " << user.phoneNumber << std::endl;
            std::cout << "Vai tro: " << User::roleToString(user.role) << std::endl;
            std::cout << "Trang thai: " << User::statusToString(user.status) << std::endl;
            std::cout << "OTP da thiet lap: " << (user.otpSecretKey.empty() ? "Chua" : "Roi") << std::endl;
            pauseScreen();
            break;
        }
        case 2: { // Cập nhật thông tin
            clearScreen();
            std::cout << "--- Cap Nhat Thong Tin Ca Nhan ---" << std::endl;
            std::string newFullName = getStringInput("Ho ten moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newFullName == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newEmail = getStringInput("Email moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newEmail == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newPhone = getStringInput("So dien thoai moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newPhone == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            
            if (newFullName.empty()) newFullName = user.fullName;
            if (newEmail.empty()) newEmail = user.email;
            if (newPhone.empty()) newPhone = user.phoneNumber;

            otpCode = "";
            if (!user.otpSecretKey.empty()) { // Nếu đã thiết lập OTP, yêu cầu OTP
                otpCode = getStringInput("Nhap ma OTP (neu da thiet lap, Go 'b' de quay lai menu): ", true);
                if (otpCode == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
            }

            if (userService.updateUserProfile(user.userId, newFullName, newEmail, newPhone, otpCode, msg)) {
                std::cout << msg << std::endl;
                auto updatedUserOpt = userService.getUserProfile(user.userId);
                if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 3: { // Đổi mật khẩu
            clearScreen();
            std::cout << "--- Doi Mat Khau ---" << std::endl;
            std::string oldPass = getStringInput("Nhap mat khau hien tai (Go 'b' de quay lai menu): ");
            if (oldPass == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newPass = getStringInput("Nhap mat khau moi (Go 'b' de quay lai menu): ");
            if (newPass == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string otpCode = "";
            std::string msg;
            
            if (!user.otpSecretKey.empty()) {
                otpCode = getStringInput("Nhap ma OTP (Go 'b' de quay lai menu): ");
                if (otpCode == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
            }
            
            if (authService.changePassword(user.userId, oldPass, newPass, otpCode, msg)) {
                std::cout << msg << std::endl;
            } else {
                std::cout << "Loi: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 4: { // Thiết lập OTP
            clearScreen();
            std::cout << "--- Thiet Lap/Xem OTP ---" << std::endl;
            if (user.otpSecretKey.empty()) {
                std::cout << "Ban chua thiet lap OTP. Ban co muon thiet lap khong? (y/n, Go 'b' de quay lai menu): ";
                std::string choiceOtp = getStringInput("");
                if (choiceOtp == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
                if (choiceOtp == "y" || choiceOtp == "Y") {
                    std::optional<std::string> secretKeyOpt = authService.setupOtpForUser(user.userId, msg);
                    if (secretKeyOpt) {
                        std::cout << msg << std::endl;
                        std::cout << "Khoa bi mat cua ban (Base32): " << secretKeyOpt.value() << std::endl;
                        std::cout << "Hay them khoa nay vao ung dung Authenticator cua ban." << std::endl;
                        std::cout << "URI (cho QR code, sao chep va dan vao trinh tao QR): " << std::endl;
                        std::cout << otpService.generateOtpUri(user.username, secretKeyOpt.value()) << std::endl;
                        // Cập nhật g_currentUser
                        auto updatedUserOpt = userService.getUserProfile(user.userId);
                        if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
                    } else {
                        std::cout << "That bai: " << msg << std::endl;
                    }
                }
            } else {
                std::cout << "OTP da duoc thiet lap." << std::endl;
                std::cout << "Khoa bi mat cua ban (Base32): " << user.otpSecretKey << std::endl;
                std::cout << "URI (cho QR code, sao chep va dan vao trinh tao QR): " << std::endl;
                std::cout << otpService.generateOtpUri(user.username, user.otpSecretKey) << std::endl;
            }
            pauseScreen();
            break;
        }
        case 5: { // Xem số dư ví
            clearScreen();
            std::cout << "--- So Du Vi ---" << std::endl;
            auto walletOpt = walletService.getWalletByUserId(user.userId);
            if (walletOpt) {
                std::cout << "So du hien tai: " << std::fixed << std::setprecision(2) << walletOpt.value().balance << " diem" << std::endl;
            } else {
                std::cout << "Khong tim thay thong tin vi. Vui long lien he ho tro." << std::endl;
            }
            pauseScreen();
            break;
        }
        case 6: { // Chuyển điểm
            clearScreen();
            std::cout << "--- Chuyen Diem ---" << std::endl;
            auto senderWalletOpt = walletService.getWalletByUserId(user.userId);
            if (!senderWalletOpt) {
                std::cout << "Loi: Khong tim thay vi cua ban." << std::endl;
                pauseScreen();
                break;
            }
            std::string receiverUsername = getStringInput("Nhap ten dang nhap cua nguoi nhan (hoac 'b' de quay lai): ");
            if (receiverUsername == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            
            auto receiverWalletOpt = walletService.getWalletByUsername(receiverUsername);
            if (!receiverWalletOpt) {
                std::cout << "Loi: Khong tim thay vi cua nguoi nhan." << std::endl;
                pauseScreen();
                break;
            }
            
            double amount = getDoubleInput("Nhap so diem muon chuyen (hoac 0 de quay lai): ");
            if (amount == 0) {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            
            otpCode = "";
            if (!user.otpSecretKey.empty()) {
                 otpCode = getStringInput("Nhap ma OTP cua ban (hoac 'b' de quay lai): ");
                 if (otpCode == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                 }
            }

            if (walletService.transferPoints(user.userId, senderWalletOpt.value().walletId, 
                                          receiverWalletOpt.value().walletId, amount, otpCode, msg)) {
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 7: { // Xem lịch sử giao dịch
            clearScreen();
            std::cout << "--- Lich Su Giao Dich ---" << std::endl;
            auto walletOpt = walletService.getWalletByUserId(user.userId);
            if (walletOpt) {
                std::vector<Transaction> history = walletService.getTransactionHistory(walletOpt.value().walletId);
                if (history.empty()) {
                    std::cout << "Khong co giao dich nao." << std::endl;
                } else {
                    for (const auto& tx : history) {
                        std::cout << "---------------------------" << std::endl;
                        std::cout << "ID Giao Dich: " << tx.transactionId << std::endl;
                        std::cout << "Thoi gian: " << TimeUtils::formatTimestamp(tx.timestamp) << std::endl;
                        std::cout << "Tu Vi: " << tx.sourceWalletId << std::endl;
                        std::cout << "Den Vi: " << tx.targetWalletId << std::endl;
                        std::cout << "So diem: " << std::fixed << std::setprecision(2) << tx.amount << std::endl;
                        std::cout << "Trang thai: " << Transaction::statusToString(tx.status) << std::endl;
                        if (!tx.description.empty()) {
                            std::cout << "Mo ta: " << tx.description << std::endl;
                        }
                    }
                    std::cout << "---------------------------" << std::endl;
                }
            } else {
                std::cout << "Khong tim thay thong tin vi." << std::endl;
            }
            pauseScreen();
            break;
        }
        case 9: // Đăng xuất
            LOG_INFO("Nguoi dung " + user.username + " dang xuat.");
            g_currentUser.reset();
            std::cout << "Da dang xuat." << std::endl;
            pauseScreen();
            return; // Quay lại vòng lặp chính (sẽ hiển thị main menu)
        case 0: // Thoát ứng dụng
            g_currentUser.reset();
            std::cout << "Thoat ung dung..." << std::endl;
            g_shouldExit = true;
            break; 
        default:
            std::cout << "Lua chon khong hop le." << std::endl;
            pauseScreen();
            break;
    }
}


void displayAdminMenu(const User& admin) {
    clearScreen();
    std::cout << "===== MENU ADMIN (" << admin.username << ") =====" << std::endl;
    std::cout << "1. Xem thong tin ca nhan (Admin)" << std::endl;
    std::cout << "2. Cap nhat thong tin ca nhan (Admin)" << std::endl;
    std::cout << "3. Doi mat khau (Admin)" << std::endl;
    std::cout << "4. Thiet lap/Xem OTP (Admin)" << std::endl;
    std::cout << "--- Quan Ly Nguoi Dung ---" << std::endl;
    std::cout << "11. Liet ke tat ca nguoi dung" << std::endl;
    std::cout << "12. Tao tai khoan nguoi dung moi" << std::endl;
    std::cout << "13. Cap nhat thong tin nguoi dung" << std::endl;
    std::cout << "14. Kich hoat tai khoan nguoi dung" << std::endl;
    std::cout << "15. Vo hieu hoa tai khoan nguoi dung" << std::endl;
    std::cout << "--- Quan Ly Vi ---" << std::endl;
    std::cout << "21. Nap diem vao vi nguoi dung" << std::endl;
    // Thêm các chức năng admin khác nếu cần
    std::cout << "9. Dang xuat" << std::endl;
    std::cout << "0. Thoat ung dung" << std::endl;
    std::cout << "===================================" << std::endl;
}

void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService, OTPService& otpService, int choice) {
    User& admin = g_currentUser.value();
    std::string msg, otpCode;

    switch (choice) {
        case 1: { // Xem thông tin cá nhân Admin
            clearScreen();
            std::cout << "--- Thong Tin Ca Nhan (Admin) ---" << std::endl;
            std::cout << "ID: " << admin.userId << std::endl;
            std::cout << "Ten dang nhap: " << admin.username << std::endl;
            std::cout << "Ho ten: " << admin.fullName << std::endl;
            std::cout << "Email: " << admin.email << std::endl;
            std::cout << "So dien thoai: " << admin.phoneNumber << std::endl;
            std::cout << "Vai tro: " << User::roleToString(admin.role) << std::endl;
            std::cout << "Trang thai: " << User::statusToString(admin.status) << std::endl;
            std::cout << "OTP da thiet lap: " << (admin.otpSecretKey.empty() ? "Chua" : "Roi") << std::endl;
            pauseScreen();
            break;
        }
        case 2: { // Cập nhật thông tin cá nhân Admin
            clearScreen();
            std::cout << "--- Cap Nhat Thong Tin Ca Nhan (Admin) ---" << std::endl;
            std::string newFullName = getStringInput("Ho ten moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newFullName == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newEmail = getStringInput("Email moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newEmail == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newPhone = getStringInput("So dien thoai moi (de trong de bo qua, Go 'b' de quay lai menu): ", true);
            if (newPhone == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            
            if (newFullName.empty()) newFullName = admin.fullName;
            if (newEmail.empty()) newEmail = admin.email;
            if (newPhone.empty()) newPhone = admin.phoneNumber;

            otpCode = "";
            if (!admin.otpSecretKey.empty()) {
                otpCode = getStringInput("Nhap ma OTP (Go 'b' de quay lai menu): ");
                if (otpCode == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
            }

            if (userService.updateUserProfile(admin.userId, newFullName, newEmail, newPhone, otpCode, msg)) {
                std::cout << msg << std::endl;
                auto updatedUserOpt = userService.getUserProfile(admin.userId);
                if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 3: { // Đổi mật khẩu Admin
            clearScreen();
            std::cout << "--- Doi Mat Khau (Admin) ---" << std::endl;
            std::string oldPass = getStringInput("Nhap mat khau hien tai (Go 'b' de quay lai menu): ");
            if (oldPass == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string newPass = getStringInput("Nhap mat khau moi (Go 'b' de quay lai menu): ");
            if (newPass == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string otpCode = "";
            
            if (!admin.otpSecretKey.empty()) {
                otpCode = getStringInput("Nhap ma OTP (Go 'b' de quay lai menu): ");
                if (otpCode == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
            }
            
            if (authService.changePassword(admin.userId, oldPass, newPass, otpCode, msg)) {
                std::cout << msg << std::endl;
            } else {
                std::cout << "Loi: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 4: { // Thiết lập OTP Admin
            clearScreen();
            std::cout << "--- Thiet Lap/Xem OTP (Admin) ---" << std::endl;
            if (admin.otpSecretKey.empty()) {
                std::cout << "Ban chua thiet lap OTP. Ban co muon thiet lap khong? (y/n, Go 'b' de quay lai menu): ";
                std::string choiceOtp = getStringInput("");
                if (choiceOtp == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    break;
                }
                if (choiceOtp == "y" || choiceOtp == "Y") {
                    std::optional<std::string> secretKeyOpt = authService.setupOtpForUser(admin.userId, msg);
                    if (secretKeyOpt) {
                        std::cout << msg << std::endl;
                        std::cout << "Khoa bi mat cua ban (Base32): " << secretKeyOpt.value() << std::endl;
                        std::cout << "Hay them khoa nay vao ung dung Authenticator cua ban." << std::endl;
                        std::cout << "URI (cho QR code, sao chep va dan vao trinh tao QR): " << std::endl;
                        std::cout << otpService.generateOtpUri(admin.username, secretKeyOpt.value()) << std::endl;
                        auto updatedUserOpt = userService.getUserProfile(admin.userId);
                        if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
                    } else {
                        std::cout << "That bai: " << msg << std::endl;
                    }
                }
            } else {
                std::cout << "OTP da duoc thiet lap." << std::endl;
                std::cout << "Khoa bi mat cua ban (Base32): " << admin.otpSecretKey << std::endl;
                std::cout << "URI (cho QR code, sao chep va dan vao trinh tao QR): " << std::endl;
                std::cout << otpService.generateOtpUri(admin.username, admin.otpSecretKey) << std::endl;
            }
            pauseScreen();
            break;
        }
        case 11: { // Liệt kê người dùng
            clearScreen();
            std::cout << "--- Danh Sach Nguoi Dung ---" << std::endl;
            std::vector<User> allUsers = adminService.listAllUsers();
            if (allUsers.empty()){
                std::cout << "Khong co nguoi dung nao trong he thong." << std::endl;
            } else {
                for (const auto& u : allUsers) {
                    std::cout << "ID: " << u.userId << ", Username: " << u.username
                              << ", Ten: " << u.fullName << ", Email: " << u.email
                              << ", Role: " << User::roleToString(u.role)
                              << ", Status: " << User::statusToString(u.status) << std::endl;
                }
            }
            pauseScreen();
            break;
        }
        case 12: { // Tạo tài khoản mới
            clearScreen();
            std::cout << "--- Admin Tao Tai Khoan Moi ---" << std::endl;
            std::string username, fullName, email, phone, tempPass;
            // Input và validate tương tự handleRegistration
            while(true){ 
                username = getStringInput("Ten dang nhap nguoi dung moi (hoac 'b' de quay lai): "); 
                if(username == "b") {
                    std::cout << "Quay lai menu truoc..." << std::endl;
                    pauseScreen();
                    return;
                }
                if(InputValidator::isValidUsername(username)) break; 
                std::cout << "Ten dang nhap khong hop le.\n"; 
            }
            fullName = getStringInput("Ho ten nguoi dung moi: ");
            while(true){ 
                email = getStringInput("Email nguoi dung moi: "); 
                if(InputValidator::isValidEmail(email)) break; 
                std::cout << "Email khong hop le.\n"; 
            }
            while(true){ 
                phone = getStringInput("So dien thoai nguoi dung moi: "); 
                if(InputValidator::isValidPhoneNumber(phone)) break; 
                std::cout << "So dien thoai khong hop le.\n"; 
            }
            // Admin chỉ nên tạo RegularUser qua giao diện này, ví dụ
            if(adminService.adminCreateUserAccount(username, fullName, email, phone, UserRole::RegularUser, tempPass, msg)){
                std::cout << "Thanh cong: " << msg << std::endl;
                std::cout << "Mat khau tam thoi cho " << username << " la: " << tempPass << std::endl;
                // Tự động tạo ví
                 User newUser; 
                for(const auto& u : g_users){ if(u.username == username){ newUser = u; break;}}
                if(!newUser.userId.empty()){
                    std::string walletMsg;
                    if(walletService.createWalletForUser(newUser.userId, walletMsg)){ std::cout << walletMsg << std::endl; }
                    else { LOG_ERROR("Tao vi that bai cho user " + newUser.username + ": " + walletMsg); std::cout << "Loi tao vi: " << walletMsg << std::endl; }
                }
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 13: { // Admin cập nhật thông tin người dùng
            clearScreen();
            std::cout << "--- Admin Cap Nhat Thong Tin Nguoi Dung ---" << std::endl;
            std::string targetUsername = getStringInput("Nhap ten dang nhap cua nguoi dung can cap nhat (Go 'b' de quay lai menu): ");
            if (targetUsername == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            auto targetUserOpt = userService.getUserByUsername(targetUsername);
            if (!targetUserOpt) {
                std::cout << "Khong tim thay nguoi dung voi ten dang nhap: " << targetUsername << std::endl;
                pauseScreen();
                break;
            }
            User targetUser = targetUserOpt.value();
            std::cout << "Cap nhat cho: " << targetUser.username << " (" << targetUser.fullName << ")" << std::endl;
            std::string newFullName = getStringInput("Ho ten moi (de trong de bo qua): ", true);
            std::string newEmail = getStringInput("Email moi (de trong de bo qua): ", true);
            std::string newPhone = getStringInput("So dien thoai moi (de trong de bo qua): ", true);
            std::cout << "Trang thai hien tai: " << User::statusToString(targetUser.status) << std::endl;
            std::cout << "Chon trang thai moi (0=NotActivated, 1=Active, 2=Inactive, de trong de bo qua): ";
            std::string statusChoiceStr = getStringInput("", true);
            
            AccountStatus newStatus = targetUser.status;
            if (!statusChoiceStr.empty()) {
                int statusChoice = -1;
                if (InputValidator::isValidInteger(statusChoiceStr, statusChoice)) {
                    if (statusChoice == 0) newStatus = AccountStatus::NotActivated;
                    else if (statusChoice == 1) newStatus = AccountStatus::Active;
                    else if (statusChoice == 2) newStatus = AccountStatus::Inactive;
                    else std::cout << "Lua chon trang thai khong hop le, trang thai se khong thay doi." << std::endl;
                } else {
                     std::cout << "Lua chon trang thai khong hop le, trang thai se khong thay doi." << std::endl;
                }
            }

            if (newFullName.empty()) newFullName = targetUser.fullName;
            if (newEmail.empty()) newEmail = targetUser.email;
            if (newPhone.empty()) newPhone = targetUser.phoneNumber;

            otpCode = "";
            if (!targetUser.otpSecretKey.empty()) { // Nếu người dùng CÓ OTP
                otpCode = getStringInput("Nhap ma OTP cua nguoi dung '" + targetUser.username + "' (do ho cung cap): ", true);
            }

            if(adminService.adminUpdateUserProfile(admin.userId, targetUser.userId, newFullName, newEmail, newPhone, newStatus, otpCode, msg)){
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 14: { // Admin kích hoạt tài khoản
            clearScreen();
            std::string targetUsername = getStringInput("Nhap ten dang nhap cua nguoi dung can kich hoat (Go 'b' de quay lai menu): ");
            if (targetUsername == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            if(authService.activateAccount(targetUsername, msg)){
                std::cout << msg << std::endl;
            } else {
                std::cout << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 15: { // Admin vô hiệu hóa tài khoản
            clearScreen();
            std::string targetUsername = getStringInput("Nhap ten dang nhap cua nguoi dung can vo hieu hoa (Go 'b' de quay lai menu): ");
            if (targetUsername == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            auto targetUserOpt = userService.getUserByUsername(targetUsername);
            if (!targetUserOpt) {
                std::cout << "Khong tim thay nguoi dung voi ten dang nhap: " << targetUsername << std::endl;
                pauseScreen();
                break;
            }
            if(adminService.adminDeactivateUser(targetUserOpt.value().userId, msg)){
                std::cout << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
         case 21: { // Admin nạp điểm
            clearScreen();
            std::cout << "--- Admin Nap Diem Vao Vi ---" << std::endl;
            std::string targetUsername = getStringInput("Nhap ten dang nhap cua nguoi dung (Go 'b' de quay lai menu): ");
            if (targetUsername == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            auto targetUserOpt = userService.getUserByUsername(targetUsername);
            if (!targetUserOpt) {
                std::cout << "Khong tim thay nguoi dung voi ten dang nhap: " << targetUsername << std::endl;
                pauseScreen();
                break;
            }
            double amount = getDoubleInput("Nhap so tien (Go 'b' de quay lai menu): ");
            if (amount == 0) {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string reason = getStringInput("Nhap ly do (Go 'b' de quay lai menu): ");
            if (reason == "b") {
                std::cout << "Quay lai menu truoc..." << std::endl;
                pauseScreen();
                break;
            }
            std::string msg;
            if (adminService.adminDepositToUserWallet(admin.userId, targetUserOpt.value().userId, amount, reason, msg)) {
                std::cout << "Nap tien thanh cong: " << msg << std::endl;
                // Show updated balance
                auto walletOpt = walletService.getWalletByUserId(targetUserOpt.value().userId);
                if (walletOpt) {
                    std::cout << "So du moi cua nguoi dung: " << std::fixed << std::setprecision(2) 
                              << walletOpt.value().balance << " diem" << std::endl;
                }
            } else {
                std::cout << "Nap tien that bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 9: // Đăng xuất
            LOG_INFO("Admin " + admin.username + " dang xuat.");
            g_currentUser.reset();
            std::cout << "Da dang xuat." << std::endl;
            pauseScreen();
            return; 
        case 0: // Thoát ứng dụng
            g_currentUser.reset();
            std::cout << "Thoat ung dung..." << std::endl;
            g_shouldExit = true;
            break;
        default:
            std::cout << "Lua chon khong hop le." << std::endl;
            pauseScreen();
            break;
    }
}