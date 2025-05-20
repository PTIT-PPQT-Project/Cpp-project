#include "Wallet.h"
#include <algorithm>

Wallet::Wallet(const std::string& id) : walletId(id), balance(0.0) {}

bool Wallet::transfer(Wallet& destination, double amount) {
    if (amount <= 0 || balance < amount) {
        return false;
    }
    
    // Create transaction record
    Transaction transaction{
        walletId,
        destination.getWalletId(),
        amount,
        std::chrono::system_clock::now(),
        false
    };
    
    // Perform transfer
    if (withdrawFunds(amount) && destination.addFunds(amount)) {
        transaction.completed = true;
        addTransaction(transaction);
        destination.addTransaction(transaction);
        return true;
    }
    
    return false;
}

bool Wallet::addFunds(double amount) {
    if (amount <= 0) {
        return false;
    }
    balance += amount;
    return true;
}

bool Wallet::withdrawFunds(double amount) {
    if (amount <= 0 || balance < amount) {
        return false;
    }
    balance -= amount;
    return true;
}

void Wallet::addTransaction(const Transaction& transaction) {
    transactionHistory.push_back(transaction);
}

std::vector<Transaction> Wallet::getTransactionsByDate(
    const std::chrono::system_clock::time_point& start,
    const std::chrono::system_clock::time_point& end) const {
    
    std::vector<Transaction> filteredTransactions;
    std::copy_if(transactionHistory.begin(), transactionHistory.end(),
                 std::back_inserter(filteredTransactions),
                 [&](const Transaction& t) {
                     return t.timestamp >= start && t.timestamp <= end;
                 });
    return filteredTransactions;
} 