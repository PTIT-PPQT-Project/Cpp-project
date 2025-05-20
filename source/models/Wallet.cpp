// src/models/Wallet.cpp
#include "../../include/models/Wallet.hpp" // Fixed include path

// Default constructor
Wallet::Wallet() :
    walletId(""),
    userId(""),
    balance(0.0),
    creationTimestamp(0),
    lastUpdateTimestamp(0) {}