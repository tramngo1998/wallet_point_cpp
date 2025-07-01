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

};

#endif

