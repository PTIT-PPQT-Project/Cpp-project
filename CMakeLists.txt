cmake_minimum_required(VERSION 3.10)
project(RewardSystem VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

# Source files
set(SOURCES
    source/main.cpp
    source/models/User.cpp
    source/models/Wallet.cpp
    source/models/Transaction.cpp
    source/services/AuthService.cpp
    source/services/UserService.cpp
    source/services/WalletService.cpp
    source/services/AdminService.cpp
    source/services/OTPService.cpp
    source/utils/FileHandler.cpp
    source/utils/HashUtils.cpp
    source/utils/InputValidator.cpp
    source/utils/Logger.cpp
    source/utils/TimeUtils.cpp
    source/utils/DataInitializer.cpp
)

# Create executable
add_executable(reward_system ${SOURCES})

# Link libraries
target_link_libraries(reward_system
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# Create data and logs directories in root
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/logs)

# Set output directories
set_target_properties(reward_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release
)

# Install targets
install(TARGETS reward_system
    RUNTIME DESTINATION bin
)

# Install data files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    DESTINATION share/reward_system
)

# Enable testing
enable_testing()

# Add compiler warnings
if(MSVC)
    target_compile_options(reward_system PRIVATE /W4)
else()
    target_compile_options(reward_system PRIVATE -Wall -Wextra -Wpedantic)
endif()