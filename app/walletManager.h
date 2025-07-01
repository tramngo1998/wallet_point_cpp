#pragma once

#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include <iostream>
#include <sqlite3.h>
#include <string>
#include <random>
#include <chrono>

using namespace std;

class WalletManager {
private:
    sqlite3* db;
	void createWalletDB();

public:
    WalletManager();
    ~WalletManager();

	string generateWalletId();
    string createWallet(const string& username, int initialBalance);
    int getBalance(const string& walletId);
    int getBalanceByUsername(const string& username);
    string getWalletIdByUsername(const string& username);
	string getUsernameByWalletId(const string& walletId);
    bool updateBalance(const string& walletId, int amount);
    bool transferPoints(const string& senderWalletId, const string& receiverWalletId, int amount);
    bool hasSufficientFunds(const string& walletId, int amount);
    bool deductFromMaster(int amount);
};

#endif

