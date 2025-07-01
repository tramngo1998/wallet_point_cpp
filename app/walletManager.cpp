#include "walletManager.h"

WalletManager::WalletManager() {
    if (sqlite3_open("data/users.db", &db) != SQLITE_OK) {
        cerr << "Khong the ket noi SQLite!" << endl;
    }
    createWalletDB();
}

WalletManager::~WalletManager() {
}

void WalletManager::createWalletDB() {
    string sql = "CREATE TABLE IF NOT EXISTS wallets ("
        "wallet_id TEXT PRIMARY KEY, "
        "username TEXT, "
        "balance INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY(username) REFERENCES users(username) ON DELETE CASCADE);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "Error creating wallets table: " << errMsg << endl;
        sqlite3_free(errMsg);
    }

    string checkMasterSql = "SELECT COUNT(*) FROM wallets WHERE username = 'master';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, checkMasterSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count == 0) {
                string insertMaster = "INSERT INTO wallets (wallet_id, username, balance) VALUES ('000000', 'master', 100000000);";
                if (sqlite3_exec(db, insertMaster.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
                    cerr << "Error creating master wallet: " << errMsg << endl;
                    sqlite3_free(errMsg);
                }
            }
        }
    }
    sqlite3_finalize(stmt);
}

