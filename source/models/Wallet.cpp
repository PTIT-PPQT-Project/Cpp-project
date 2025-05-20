// src/models/Wallet.cpp
#include "include/models/Wallet.hpp" // Điều chỉnh đường dẫn nếu cần

// Default constructor
Wallet::Wallet() :
    walletId(""),
    userId(""),
    balance(0.0),
    creationTimestamp(0),
    lastUpdateTimestamp(0) {}