cmake_minimum_required(VERSION 3.14) # nlohmann/json khuyến nghị 3.14+ cho FetchContent tốt hơn

project(RewardSystem VERSION 1.0 DESCRIPTION "Hệ thống đăng nhập, đăng ký và ví điểm thưởng")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Tích hợp nlohmann/json bằng FetchContent ---
include(FetchContent)
FetchContent_Declare(
    nlohmann_json           # Tên để gọi trong CMake
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3         # Thay bằng phiên bản bạn muốn, ví dụ tag mới nhất
)
FetchContent_MakeAvailable(nlohmann_json)
# Sau lệnh này, bạn có thể dùng target `nlohmann_json::nlohmann_json`
# hoặc thư viện sẽ tự động được thêm vào include path nếu nó là header-only
# và được thiết kế để hoạt động tốt với FetchContent.
# Đối với nlohmann/json (header-only), nó thường sẽ thêm include directory cần thiết.

# ... (Phần còn lại của CMakeLists.txt) ...

set(APP_SOURCES
    source/main.cpp
    source/models/User.cpp
    source/models/Wallet.cpp
    source/models/Transaction.cpp
    source/utils/FileHandler.cpp
    source/utils/HashUtils.cpp
    source/utils/InputValidator.cpp
    source/utils/Logger.cpp
    source/utils/TimeUtils.cpp
    source/services/OTPService.cpp
    source/services/AuthService.cpp
    source/services/UserService.cpp
    source/services/WalletService.cpp
    source/services/AdminService.cpp
)

add_executable(RewardSystemApp ${APP_SOURCES})

target_include_directories(RewardSystemApp PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    # Nếu FetchContent không tự thêm, bạn có thể cần:
    # ${nlohmann_json_SOURCE_DIR}/include # Hoặc đường dẫn tương tự mà FetchContent cung cấp
)

# Nếu nlohmann_json tạo ra một target library (thường là cho các thư viện không phải header-only), bạn sẽ link nó:
# target_link_libraries(RewardSystemApp PRIVATE nlohmann_json::nlohmann_json)
# Tuy nhiên, với header-only và FetchContent, việc include thường được xử lý tự động.
# Chỉ cần đảm bảo nlohmann_json_SOURCE_DIR/include được thêm vào include path.
# Thư viện nlohmann/json thường tự xử lý việc này khi dùng với CMake.