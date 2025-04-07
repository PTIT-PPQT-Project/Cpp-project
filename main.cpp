#include <iostream>
#include <unordered_map>
#include "include/UserAccount.h"
#include "include/Wallet.h"
#include "include/Utils.h"

std::unordered_map<std::string, UserAccount> userDB;
std::unordered_map<std::string, Wallet> walletDB;

void registerUser() {
    clearConsole();
    std::string username, password, name, email, phone;
    bool isManager = false;

    std::cout << "--- Đăng ký tài khoản ---\n";
    std::cout << "Tên người dùng: "; std::cin >> username;

    if (userDB.find(username) != userDB.end()) {
        std::cout << "Tên người dùng đã tồn tại!\n";
        pause(); return;
    }

    std::cout << "Họ và tên: "; std::cin.ignore(); std::getline(std::cin, name);
    std::cout << "Email: "; std::getline(std::cin, email);
    std::cout << "Số điện thoại: "; std::getline(std::cin, phone);

    std::cout << "Nhập mật khẩu (hoặc để trống để sinh tự động): ";
    std::getline(std::cin, password);

    bool autoPwd = password.empty();
    if (autoPwd) password = generatePassword();

    std::string hashed = hashPassword(password);
    std::string walletID = "WLT_" + username;

    UserAccount user(username, hashed, name, email, phone, isManager, autoPwd, walletID);
    Wallet wallet(walletID);

    userDB[username] = user;
    walletDB[walletID] = wallet;

    std::cout << "\nTài khoản đã được tạo.\n";
    if (autoPwd)
        std::cout << "Mật khẩu tự sinh: " << password << "\n";
    pause();
}

void loginUser() {
    clearConsole();
    std::string username, password;
    std::cout << "--- Đăng nhập ---\n";
    std::cout << "Tên người dùng: "; std::cin >> username;
    std::cout << "Mật khẩu: "; std::cin >> password;

    auto it = userDB.find(username);
    if (it == userDB.end()) {
        std::cout << "Tài khoản không tồn tại!\n"; pause(); return;
    }

    std::string hashed = hashPassword(password);
    if (hashed != it->second.hashedPassword) {
        std::cout << "Sai mật khẩu!\n"; pause(); return;
    }

    std::cout << "\nĐăng nhập thành công!\n";
    std::cout << "Chào mừng, " << it->second.fullName << "!\n";
    pause();
}

int main() {
    srand(time(NULL));
    int choice;
    do {
        clearConsole();
        std::cout << "========= HỆ THỐNG ĐIỂM THƯởNG =========\n";
        std::cout << "1. Đăng ký\n";
        std::cout << "2. Đăng nhập\n";
        std::cout << "0. Thoát\n";
        std::cout << "Chọn: ";
        std::cin >> choice;

        switch (choice) {
            case 1: registerUser(); break;
            case 2: loginUser(); break;
            case 0: std::cout << "Tạm biệt!\n"; break;
            default: std::cout << "Lựa chọn không hợp lệ!\n"; pause(); break;
        }
    } while (choice != 0);

    return 0;
}