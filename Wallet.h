#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include <chrono>

struct Transaction {
    std::string fromWalletId;
    std::string toWalletId;
    double amount;
    std::chrono::system_clock::time_point timestamp;
    bool completed;
};

class Wallet {
private:
    std::string walletId;
    double balance;
    std::vector<Transaction> transactionHistory;
    
public:
    Wallet(const std::string& id);
    
    // Getters
    std::string getWalletId() const { return walletId; }
    double getBalance() const { return balance; }
    const std::vector<Transaction>& getTransactionHistory() const { return transactionHistory; }
    
    // Transaction operations
    bool transfer(Wallet& destination, double amount);
    bool addFunds(double amount);
    bool withdrawFunds(double amount);
    
    // Transaction history
    void addTransaction(const Transaction& transaction);
    std::vector<Transaction> getTransactionsByDate(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) const;
};

#endif 