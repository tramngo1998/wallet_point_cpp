#include "transactionManager.h"

TransactionManager::TransactionManager() {
    if (sqlite3_open("data/users.db", &db) != SQLITE_OK) {
        cerr << "Cannot open SQLite in TransactionManager!" << endl;
    }
    createTransactionTable();
}

TransactionManager::~TransactionManager() {
}

void TransactionManager::createTransactionTable() {
    string sql = "CREATE TABLE IF NOT EXISTS transactions ("
        "transaction_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "sender TEXT, "
        "senderDisplay TEXT, "
        "receiver TEXT, "
        "receiverDisplay TEXT, "
        "amount INTEGER, "
        "type TEXT, "
        "status TEXT, "
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        cerr << "Error creating transactions table: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
}

void TransactionManager::recordTransaction(const string& senderWalletId, const string& senderDisplay,
    const string& receiverWalletId, const string& receiverDisplay,
    int amount, const string& type, const string& status) {
    string sql = "INSERT INTO transactions (sender, senderDisplay, receiver, receiverDisplay, amount, type, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, senderWalletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, senderDisplay.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, receiverWalletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, receiverDisplay.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, amount);
        sqlite3_bind_text(stmt, 6, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, status.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error recording transaction: " << sqlite3_errmsg(db) << endl;
        }
    }
    else {
        cerr << "Error preparing statement for transaction: " << sqlite3_errmsg(db) << endl;
    }
    sqlite3_finalize(stmt);
}

vector<string> TransactionManager::getTransactionHistory(const string& walletId) {
    vector<string> history;
    string sql = "SELECT transaction_id, sender, senderDisplay, receiver, receiverDisplay, amount, type, status, timestamp "
        "FROM transactions WHERE sender = ? OR receiver = ? ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        // Bind walletId for both sender and receiver
        sqlite3_bind_text(stmt, 1, walletId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, walletId.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int transId = sqlite3_column_int(stmt, 0);
            string sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string senderDisplay = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            string receiver = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            string receiverDisplay = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            int amount = sqlite3_column_int(stmt, 5);
            string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

            // Format the record including display names.
            string record = "ID: " + to_string(transId) + " | " + timestamp +
                " | From: " + senderDisplay + " (" + sender + ")" +
                " -> To: " + receiverDisplay + " (" + receiver + ")" +
                " | Amount: " + to_string(amount) +
                " | Type: " + type +
                " | Status: " + status;
            history.push_back(record);
        }
    }
    else {
        cerr << "Error preparing transaction history query: " << sqlite3_errmsg(db) << endl;
    }
    sqlite3_finalize(stmt);
    return history;
}