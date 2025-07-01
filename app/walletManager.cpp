#include "walletManager.h"

WalletManager::WalletManager() {
    if (sqlite3_open("data/users.db", &db) != SQLITE_OK) {
        cerr << "Khong the ket noi SQLite!" << endl;
    }
    createWalletDB();
}

WalletManager::~WalletManager() {
}

string WalletManager::generateWalletId() {
    auto now = chrono::system_clock::now();
    auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(1000, 9999);
    int randomNum = dis(gen);

    stringstream ss;
    ss << "ACC" << (timestamp % 1000000) << randomNum; 

    return ss.str();
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

string WalletManager::createWallet(const string& username, int initialBalance) {
    string walletId = generateWalletId();
    string sql = "INSERT INTO wallets (wallet_id, username, balance) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, initialBalance);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return walletId;
        }
    }
    cerr << "Error creating wallet: " << sqlite3_errmsg(db) << endl;
    sqlite3_finalize(stmt);
    return "";
}

int WalletManager::getBalance(const string& walletId) {
    string sql = "SELECT balance FROM wallets WHERE wallet_id = ?;";
    sqlite3_stmt* stmt;
    int balance = -1;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            balance = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return balance;
}

int WalletManager::getBalanceByUsername(const string& username) {
    string sql = "SELECT balance FROM wallets WHERE username = ?;";
    sqlite3_stmt* stmt;
    int balance = -1;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            balance = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return balance;
}

string WalletManager::getWalletIdByUsername(const string& username) {
    string sql = "SELECT wallet_id FROM wallets WHERE username = ?;";
    sqlite3_stmt* stmt;
    string walletId;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* idText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (idText) {
                walletId = idText;
            }
        }
    }
    sqlite3_finalize(stmt);
    return walletId;
}

string WalletManager::getUsernameByWalletId(const string& walletId) {
    string sql = "SELECT username FROM wallets WHERE wallet_id = ?;";
    sqlite3_stmt* stmt;
    string username;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (text)
                username = text;
        }
    }
    sqlite3_finalize(stmt);
    return username;
}

bool WalletManager::updateBalance(const string& walletId, int amount) {
    int currentBalance = getBalance(walletId);
    if (currentBalance == -1) {
        cerr << "Loi: Tai khoan khong ton tai!" << endl;
        return false;
    }
    if (currentBalance + amount < 0) {
        cerr << "Loi: So du khong du!" << endl;
        return false;
    }
    string sql = "UPDATE wallets SET balance = balance + ? WHERE wallet_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, amount);
        sqlite3_bind_text(stmt, 2, walletId.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        }
    }
    cerr << "Loi khi cap nhat so du: " << sqlite3_errmsg(db) << endl;
    sqlite3_finalize(stmt);
    return false;
}

bool WalletManager::transferPoints(const string& senderWalletId, const string& receiverWalletId, int amount) {
    if (!hasSufficientFunds(senderWalletId, amount)) {
        cerr << "Loi: So du nguoi gui khong du!" << endl;
        return false;
    }
    if (!updateBalance(senderWalletId, -amount)) {
        cerr << "Loi: Khong the tru so du cua nguoi gui!" << endl;
        return false;
    }
    if (!updateBalance(receiverWalletId, amount)) {
        cerr << "Loi: Khong the cong so du cho nguoi nhan!" << endl;
        updateBalance(senderWalletId, amount); 
        return false;
    }
    return true;
}

bool WalletManager::hasSufficientFunds(const string& walletId, int amount) {
    int balance = getBalance(walletId);
    return balance >= amount;
}

bool WalletManager::deductFromMaster(int amount) {
    string masterWalletId = "000000";
    int masterBalance = getBalance(masterWalletId);
    if (masterBalance >= amount) {
        return updateBalance(masterWalletId, -amount);
    }
    cerr << "Loi: So du master khong du!" << endl;
    return false;
}


