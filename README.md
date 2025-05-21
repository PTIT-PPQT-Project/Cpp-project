# Hệ Thống Quản Lý Điểm Thưởng (Reward System)

## 1. Giới Thiệu Dự Án

Hệ thống Quản lý Điểm Thưởng là một ứng dụng C++ được thiết kế để quản lý và xử lý các giao dịch điểm thưởng giữa người dùng. Hệ thống cung cấp các chức năng đăng ký, đăng nhập, quản lý ví điểm, và thực hiện các giao dịch điểm thưởng với các tính năng bảo mật như xác thực OTP.

### 1.1 Tính Năng Chính
- Quản lý người dùng (đăng ký, đăng nhập, cập nhật thông tin)
- Quản lý ví điểm (tạo ví, nạp điểm, chuyển điểm)
- Xác thực hai yếu tố (OTP)
- Quản lý giao dịch và lịch sử
- Phân quyền người dùng (Admin/User)

## 2. Thành Viên Dự Án

[Để trống - Sẽ cập nhật sau]

## 3. Phân Tích và Đặc Tả Chức Năng

### 3.1 Cấu Trúc Hệ Thống

```
Cpp-project/
├── build/                  # Thư mục build
├── data/                   # Dữ liệu ứng dụng
│   ├── users.json         # Thông tin người dùng
│   ├── wallets.json       # Thông tin ví
│   └── transactions.json  # Lịch sử giao dịch
├── include/               # Header files
│   ├── models/           # Các model dữ liệu
│   ├── services/         # Logic nghiệp vụ
│   └── utils/            # Tiện ích
├── source/               # Source code
├── logs/                 # Log files
└── CMakeLists.txt        # Cấu hình CMake
```

### 3.2 Các Module Chính

#### 3.2.1 Models
- **User**: Quản lý thông tin người dùng
- **Wallet**: Quản lý ví điểm
- **Transaction**: Quản lý giao dịch

#### 3.2.2 Services
- **AuthService**: Xác thực và phân quyền
- **UserService**: Quản lý người dùng
- **WalletService**: Quản lý ví và giao dịch
- **AdminService**: Chức năng quản trị
- **OTPService**: Xác thực hai yếu tố

#### 3.2.3 Utils
- **FileHandler**: Xử lý file JSON
- **HashUtils**: Mã hóa và bảo mật
- **Logger**: Ghi log
- **TimeUtils**: Xử lý thời gian
- **InputValidator**: Kiểm tra dữ liệu đầu vào

### 3.3 Quy Trình Xử Lý

#### 3.3.1 Đăng Ký Người Dùng
1. Kiểm tra thông tin đầu vào
2. Mã hóa mật khẩu
3. Tạo tài khoản mới
4. Tạo ví điểm mặc định
5. Lưu thông tin vào file

#### 3.3.2 Giao Dịch Điểm
1. Xác thực người dùng
2. Kiểm tra số dư
3. Thực hiện giao dịch
4. Cập nhật số dư
5. Ghi log giao dịch

## 4. Hướng Dẫn Cài Đặt và Chạy

### 4.1 Yêu Cầu Hệ Thống
- C++17 trở lên
- CMake 3.10 trở lên
- Trình biên dịch hỗ trợ C++17
- Git

### 4.2 Thư Viện Cần Thiết
- nlohmann/json: Xử lý JSON
- OpenSSL: Chức năng OTP

### 4.3 Cài Đặt

#### Windows
```bash
# Cài đặt vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Cài đặt thư viện
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install openssl:x64-windows

# Tích hợp với Visual Studio
.\vcpkg integrate install
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev
```

#### macOS
```bash
brew install cmake openssl
```

### 4.4 Build Project
```bash
# Tạo thư mục build
mkdir build
cd build

# Cấu hình project
# Windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
# Linux/macOS
cmake ..

# Build
# Windows
cmake --build . --config Release
# Linux/macOS
make
```

### 4.5 Chạy Chương Trình

#### Windows
```bash
.\build\Release\reward_system.exe
```

#### Linux/macOS
```bash
./build/reward_system
```

## 5. Hướng Dẫn Sử Dụng

### 5.1 Menu Chính
1. Đăng nhập
2. Đăng ký
3. Thoát

### 5.2 Menu Người Dùng
1. Xem thông tin cá nhân
2. Cập nhật thông tin
3. Đổi mật khẩu
4. Thiết lập OTP
5. Xem số dư
6. Chuyển điểm
7. Xem lịch sử giao dịch
8. Đăng xuất

### 5.3 Menu Admin 
1. Xem danh sách người dùng
2. Tạo tài khoản mới
3. Cập nhật thông tin người dùng
4. Kích hoạt/vô hiệu hóa tài khoản
5. Nạp điểm cho người dùng
6. Đăng xuất

## 6. Tài Liệu Tham Khảo

1. [nlohmann/json Documentation](https://github.com/nlohmann/json)
2. [OpenSSL Documentation](https://www.openssl.org/docs/)
3. [CMake Documentation](https://cmake.org/documentation/)
4. [C++17 Standard](https://isocpp.org/std/the-standard)

## 7. Ghi Chú Phát Triển

### 7.1 Các Vấn Đề Đã Biết
- Cần hoàn thiện xử lý lỗi và rollback trong các thao tác file
- Cần tích hợp thư viện OTP thực tế
- Cần bổ sung kiểm tra quyền truy cập chi tiết

### 7.2 Hướng Phát Triển
- Thêm tính năng báo cáo thống kê
- Cải thiện giao diện người dùng
- Tối ưu hóa hiệu suất xử lý giao dịch
- Thêm tính năng sao lưu và khôi phục dữ liệu
