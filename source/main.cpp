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

// Forward declarations for handler functions (now the types should be known)
void displayMainMenu();
void displayUserMenu(const User& user); // User type is known from User.hpp
void displayAdminMenu(const User& admin); // User type is known

// Prototypes for handler functions - types like AuthService, WalletService should now be known
void handleRegistration(AuthService& authService, WalletService& walletService);
void handleLogin(AuthService& authService);
void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService);
void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService);

// Utility input functions
std::string getStringInput(const std::string& prompt, bool allowEmpty = false);
int getIntInput(const std::string& prompt);
double getDoubleInput(const std::string& prompt);
void clearScreen();
void pauseScreen();


int main() {
    // 1. Khởi tạo Logger
    // Cấu hình logger một lần (có thể đặt tên file log khác hoặc mức log khác)
    // Logger sẽ tự tạo thư mục "logs" nếu chưa có, dựa vào EnsureDirectoryForFileExists
    Logger::getInstance("logs/app.log", LogLevel::INFO, LogLevel::DEBUG);
    LOG_INFO("Ung dung khoi dong.");

    // 2. Khởi tạo các Utilities và Services
    HashUtils hashUtils; // HashUtils có thông báo WARNING về demo hashing trong constructor
    FileHandler fileHandler("data/"); // Dữ liệu sẽ lưu trong thư mục "data/"
    OTPService otpService;

    // 3. Tải dữ liệu ban đầu
    LOG_INFO("Dang tai du lieu...");
    if (!fileHandler.loadUsers(g_users)) {
        LOG_ERROR("Khong the tai du lieu nguoi dung. Co the file bi loi hoac khong ton tai.");
        // Có thể quyết định dừng ứng dụng ở đây nếu dữ liệu người dùng là thiết yếu
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


    AuthService authService(g_users, fileHandler, otpService, hashUtils);
    UserService userService(g_users, fileHandler, otpService);
    WalletService walletService(g_users, g_wallets, g_transactions, fileHandler, otpService);
    AdminService adminService(g_users, authService, userService, walletService);

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
        if (!g_currentUser.has_value()) { // Chưa đăng nhập
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
                handleAdminActions(adminService, userService, authService, walletService);
            } else {
                handleUserActions(userService, authService, walletService);
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
    std::string username, password, fullName, email, phone;
    
    while(true){
        username = getStringInput("Ten dang nhap (3-20 ky tu, alphanumeric, _): ");
        if(InputValidator::isValidUsername(username)) break;
        std::cout << "Ten dang nhap khong hop le.\n";
    }
    while(true){
        password = getStringInput("Mat khau (min 8 ky tu, co chu hoa, thuong, so, ky tu dac biet): ");
        if(InputValidator::isValidPassword(password)) break;
        std::cout << "Mat khau khong du manh.\n";
    }
    fullName = getStringInput("Ho ten day du: ");
     while(true){
        email = getStringInput("Email: ");
        if(InputValidator::isValidEmail(email)) break;
        std::cout << "Email khong hop le.\n";
    }
    while(true){
        phone = getStringInput("So dien thoai: ");
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
            if(walletService.createWalletForUser(newUser.userId, walletMsg)){
                std::cout << walletMsg << std::endl;
            } else {
                LOG_ERROR("Tao vi that bai cho nguoi dung " + newUser.username + ": " + walletMsg);
                std::cout << "Loi khi tao vi: " << walletMsg << std::endl;
            }
        } else {
            LOG_ERROR("Khong tim thay nguoi dung " + username + " sau khi dang ky de tao vi.");
        }
    } else {
        std::cout << "Dang ky that bai: " << msg << std::endl;
    }
    pauseScreen();
}

void handleLogin(AuthService& authService) {
    clearScreen();
    std::cout << "--- Dang Nhap ---" << std::endl;
    std::string username = getStringInput("Ten dang nhap: ");
    std::string password = getStringInput("Mat khau: ");
    std::string msg;
    std::optional<User> userOpt = authService.loginUser(username, password, msg);
    if (userOpt) {
        g_currentUser = userOpt.value();
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

void handleUserActions(UserService& userService, AuthService& authService, WalletService& walletService) {
    User& user = g_currentUser.value(); // Lấy tham chiếu để có thể cập nhật (ví dụ: sau khi đổi mk)
    displayUserMenu(user);
    int choice = getIntInput("Lua chon cua ban: ");
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
            std::string newFullName = getStringInput("Ho ten moi (de trong de bo qua): ", true);
            std::string newEmail = getStringInput("Email moi (de trong de bo qua): ", true);
            std::string newPhone = getStringInput("So dien thoai moi (de trong de bo qua): ", true);
            
            if (newFullName.empty()) newFullName = user.fullName;
            if (newEmail.empty()) newEmail = user.email;
            if (newPhone.empty()) newPhone = user.phoneNumber;

            otpCode = "";
            if (!user.otpSecretKey.empty()) { // Nếu đã thiết lập OTP, yêu cầu OTP
                otpCode = getStringInput("Nhap ma OTP (neu da thiet lap): ", true);
            }

            if (userService.updateUserProfile(user.userId, newFullName, newEmail, newPhone, otpCode, msg)) {
                std::cout << "Thanh cong: " << msg << std::endl;
                // Cập nhật lại g_currentUser nếu thông tin thay đổi được lưu trong vector g_users
                // Cách tốt hơn là userService trả về User đã cập nhật hoặc AuthService cập nhật trực tiếp g_currentUser
                 auto updatedUserOpt = userService.getUserProfile(user.userId); // Tải lại từ vector
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
            std::string oldPass = getStringInput("Mat khau cu: ");
            std::string newPass, confirmPass;
             while(true){
                newPass = getStringInput("Mat khau moi (min 8 ky tu, phức tạp): ");
                if(InputValidator::isValidPassword(newPass)) break;
                std::cout << "Mat khau khong du manh.\n";
            }
            confirmPass = getStringInput("Xac nhan mat khau moi: ");
            if (newPass == confirmPass) {
                if (authService.changePassword(user.userId, oldPass, newPass, msg)) {
                    std::cout << "Thanh cong: " << msg << std::endl;
                     auto updatedUserOpt = userService.getUserProfile(user.userId); // Tải lại từ vector
                     if(updatedUserOpt) g_currentUser = updatedUserOpt.value();
                } else {
                    std::cout << "That bai: " << msg << std::endl;
                }
            } else {
                std::cout << "Mat khau xac nhan khong khop." << std::endl;
            }
            pauseScreen();
            break;
        }
        case 4: { // Thiết lập OTP
            clearScreen();
            std::cout << "--- Thiet Lap/Xem OTP ---" << std::endl;
            if (user.otpSecretKey.empty()) {
                std::cout << "Ban chua thiet lap OTP. Ban co muon thiet lap khong? (y/n): ";
                std::string choiceOtp = getStringInput("");
                if (choiceOtp == "y" || choiceOtp == "Y") {
                    std::optional<std::string> secretKeyOpt = authService.setupOtpForUser(user.userId, msg);
                    if (secretKeyOpt) {
                        std::cout << msg << std::endl;
                        std::cout << "Khoa bi mat cua ban (Base32): " << secretKeyOpt.value() << std::endl;
                        std::cout << "Hay them khoa nay vao ung dung Authenticator cua ban." << std::endl;
                        std::cout << "URI (cho QR code, sao chep va dan vao trinh tao QR): " << std::endl;
                        std::cout << otpService.generateOtpUri(user.username, secretKeyOpt.value(), "RewardApp") << std::endl;
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
                 std::cout << otpService.generateOtpUri(user.username, user.otpSecretKey, "RewardApp") << std::endl;
                // Không hiển thị lại secret key ở đây trừ khi người dùng yêu cầu explicit và xác thực lại.
                // Hoặc cung cấp tùy chọn "Vô hiệu hóa OTP"
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
            std::string receiverWalletId = getStringInput("Nhap ID vi nguoi nhan: ");
            double amount = getDoubleInput("Nhap so diem muon chuyen: ");
            
            otpCode = "";
            if (!user.otpSecretKey.empty()) {
                 otpCode = getStringInput("Nhap ma OTP cua ban: ");
            }

            if (walletService.transferPoints(user.userId, senderWalletOpt.value().walletId, receiverWalletId, amount, otpCode, msg)) {
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
                        std::cout << "Thoi gian: " << TimeUtils::formatTimestamp(tx.transactionTimestamp) << std::endl;
                        std::cout << "Tu Vi: " << tx.senderWalletId << std::endl;
                        std::cout << "Den Vi: " << tx.receiverWalletId << std::endl;
                        std::cout << "So diem: " << std::fixed << std::setprecision(2) << tx.amountTransferred << std::endl;
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
            g_currentUser.reset(); // Đảm bảo đăng xuất trước khi thoát
            // running được quản lý ở vòng lặp ngoài, nên cần cách để báo hiệu thoát
            // Trong trường hợp này, main() sẽ thoát khi handleUserActions/handleAdminActions kết thúc và lựa chọn là 0
            // Để thoát hẳn, main loop cần biến running=false
            // Tạm thời:
            std::cout << "Thoat ung dung..." << std::endl;
            exit(0); // Thoát ngay lập tức (không lý tưởng bằng cờ running)
            break; // Sẽ không bao giờ tới đây nếu exit(0) được gọi
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

void handleAdminActions(AdminService& adminService, UserService& userService, AuthService& authService, WalletService& walletService) {
    User& admin = g_currentUser.value(); // Lấy tham chiếu
    displayAdminMenu(admin);
    int choice = getIntInput("Lua chon cua ban: ");
    std::string msg, otpCode;

    // Các case 1-4 giống hệt user menu, có thể tách thành hàm chung
    // hoặc gọi lại handleUserActions với một số quyền hạn đặc biệt nếu cần
    switch (choice) {
        case 1: case 2: case 3: case 4: { // Chức năng cá nhân của Admin
            // Tạm thời chuyển sang menu người dùng cho các chức năng này
            // Để đơn giản, chúng ta sẽ gọi lại handleUserActions,
            // nhưng trong thực tế, bạn có thể muốn các hàm riêng cho admin tự quản lý mình.
            // Hoặc tạo một hàm handleSelfCareActions(user, services...)
            std::cout << "Chuc nang nay tuong tu nhu menu nguoi dung." << std::endl;
            handleUserActions(userService, authService, walletService); // Gọi lại hàm của user
            // Cần lưu ý là sau khi hàm này chạy xong, nó có thể đã thay đổi g_currentUser.
            // Nếu g_currentUser bị reset (logout), vòng lặp chính sẽ xử lý.
            return; // Quay lại vòng lặp chính để kiểm tra g_currentUser
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
            while(true){ username = getStringInput("Ten dang nhap nguoi dung moi: "); if(InputValidator::isValidUsername(username)) break; std::cout << "Ten dang nhap khong hop le.\n"; }
            fullName = getStringInput("Ho ten nguoi dung moi: ");
            while(true){ email = getStringInput("Email nguoi dung moi: "); if(InputValidator::isValidEmail(email)) break; std::cout << "Email khong hop le.\n"; }
            while(true){ phone = getStringInput("So dien thoai nguoi dung moi: "); if(InputValidator::isValidPhoneNumber(phone)) break; std::cout << "So dien thoai khong hop le.\n"; }
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
            std::string targetUserId = getStringInput("Nhap User ID cua nguoi dung can cap nhat: ");
            auto targetUserOpt = userService.getUserProfile(targetUserId);
            if (!targetUserOpt) {
                std::cout << "Khong tim thay nguoi dung voi ID: " << targetUserId << std::endl;
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

            if(adminService.adminUpdateUserProfile(admin.userId, targetUserId, newFullName, newEmail, newPhone, newStatus, otpCode, msg)){
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 14: { // Admin kích hoạt tài khoản
            clearScreen();
            std::string targetUserId = getStringInput("Nhap User ID cua nguoi dung can kich hoat: ");
            if(adminService.adminActivateUser(targetUserId, msg)){
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
        case 15: { // Admin vô hiệu hóa tài khoản
            clearScreen();
            std::string targetUserId = getStringInput("Nhap User ID cua nguoi dung can vo hieu hoa: ");
            if(adminService.adminDeactivateUser(targetUserId, msg)){
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
            }
            pauseScreen();
            break;
        }
         case 21: { // Admin nạp điểm
            clearScreen();
            std::cout << "--- Admin Nap Diem Vao Vi ---" << std::endl;
            std::string targetUserId = getStringInput("Nhap User ID cua nguoi dung can nap diem: ");
            double amount = getDoubleInput("Nhap so diem can nap: ");
            std::string reason = getStringInput("Nhap ly do nap diem: ");
            if (adminService.adminDepositToUserWallet(targetUserId, amount, reason, msg)) {
                std::cout << "Thanh cong: " << msg << std::endl;
            } else {
                std::cout << "That bai: " << msg << std::endl;
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
            exit(0);
            break;
        default:
            std::cout << "Lua chon khong hop le." << std::endl;
            pauseScreen();
            break;
    }
}