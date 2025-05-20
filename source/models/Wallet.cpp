// src/models/Wallet.cpp
#include "../include/models/Wallet.hpp" // Adjust path to be relative to source/models directory

// Default constructor
Wallet::Wallet() :
    walletId(""),
    userId(""),
    balance(0.0),
    creationTimestamp(0),
    lastUpdateTimestamp(0) {}