#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <unordered_map>
#include "model/UserAccount.h"
#include "model/Wallet.h"

void registerUser(std::unordered_map<std::string, UserAccount>& userDB,
                  std::unordered_map<std::string, Wallet>& walletDB);
void loginUser(std::unordered_map<std::string, UserAccount>& userDB,
               std::unordered_map<std::string, Wallet>& walletDB);
void loadUserDB(std::unordered_map<std::string, UserAccount>& userDB);
void saveUserDB(const std::unordered_map<std::string, UserAccount>& userDB);
void loadWalletDB(std::unordered_map<std::string, Wallet>& walletDB);
void saveWalletDB(const std::unordered_map<std::string, Wallet>& walletDB);

#endif