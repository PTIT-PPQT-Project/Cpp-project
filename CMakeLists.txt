cmake_minimum_required(VERSION 3.10)
project(WalletSystem)

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(wallet_system
    main.cpp
    User.cpp
    UserManager.cpp
    OTPManager.cpp
    Wallet.cpp
)

# Link OpenSSL
target_link_libraries(wallet_system PRIVATE OpenSSL::SSL OpenSSL::Crypto) 