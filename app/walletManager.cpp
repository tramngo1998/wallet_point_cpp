#include "walletManager.h"

WalletManager::WalletManager() {
    if (sqlite3_open("data/users.db", &db) != SQLITE_OK) {
        cerr << "Khong the ket noi SQLite!" << endl;
    }
    createWalletDB();
}

WalletManager::~WalletManager() {
}

