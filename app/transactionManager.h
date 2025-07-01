#pragma once
#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class TransactionManager {
private:
    sqlite3* db;
    void createTransactionTable();
public:
    TransactionManager();
    ~TransactionManager();

    void recordTransaction(const string& senderWalletId, const string& senderDisplay,
        const string& receiverWalletId, const string& receiverDisplay,
        int amount, const string& type, const string& status);
    vector<string> getTransactionHistory(const string& username);
};

#endif