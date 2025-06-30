#include "userDatabase.h"


UserDatabase::UserDatabase() {
    if (sqlite3_open("data/users.db", &db) != SQLITE_OK) {
        cerr << "Khong the ket noi SQLite!" << endl;
    }
    initializeDatabase();
}

UserDatabase::~UserDatabase() {
    sqlite3_close(db);
}

void UserDatabase::initializeDatabase() {
    string sql = "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY, "
        "password TEXT, "
        "role TEXT, "
        "fullName TEXT, "
        "phoneNumber TEXT, "
        "mustChangePassword INTEGER DEFAULT 0);";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    string sqlPending = "CREATE TABLE IF NOT EXISTS pending_changes ("
        "username TEXT PRIMARY KEY, "
        "newFullName TEXT, "
        "newPhoneNumber TEXT, "
        "status TEXT DEFAULT 'pending', "
        "FOREIGN KEY(username) REFERENCES users(username));";
    sqlite3_exec(db, sqlPending.c_str(), nullptr, nullptr, nullptr);

    string checkAdminSql = "SELECT COUNT(*) FROM users WHERE role='manager';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, checkAdminSql.c_str(), -1, &stmt, 0);

    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) == 0) {
        string adminPassword = "admin123";  // Mật khẩu mặc định
        string hashedAdminPass = hashPassword(adminPassword); // Băm mật khẩu

        string insertAdminSql = "INSERT INTO users (username, password, role, fullName, phoneNumber, mustChangePassword) VALUES "
            "('admin', '" + hashedAdminPass + "', 'manager', 'Admin User', '0123456789', 0);";
        sqlite3_exec(db, insertAdminSql.c_str(), 0, 0, 0);
    }
    sqlite3_finalize(stmt);
}

bool UserDatabase::userExists(const string& username) {
    string sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count > 0;
}

string UserDatabase::hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool UserDatabase::verifyPassword(const string& inputPassword, const string& storedHashedPassword) {
    return hashPassword(inputPassword) == storedHashedPassword;
}

User UserDatabase::getUser(const string& username) {
    User user;
    string sql = "SELECT username, password, role, fullName, phoneNumber, mustChangePassword FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            user.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            user.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            user.phoneNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            user.mustChangePassword = sqlite3_column_int(stmt, 5);
        }
        else {
            cerr << "Nguoi dung khong ton tai!" << endl;
        }
        sqlite3_finalize(stmt);
    }
    else {
        cerr << "Loi khi truy van SQLite: " << sqlite3_errmsg(db) << endl;
    }

    return user;
}

void UserDatabase::addUser(const string& username, const string& password, const string& role, const string& fullName, const string& phoneNumber, int mustChangePassword) {
    string hashedPassword = hashPassword(password);  // Băm mật khẩu
    string sql = "INSERT INTO users (username, password, role, fullName, phoneNumber, mustChangePassword) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, fullName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, phoneNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, mustChangePassword);  // Thêm tham số mustChangePassword

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Loi trong qua trinh them nguoi dung: " << sqlite3_errmsg(db) << endl;
        }
        sqlite3_finalize(stmt);
    }
    else {
        cerr << "Loi khi prepare statement: " << sqlite3_errmsg(db) << endl;
    }
}

void UserDatabase::updateUserPassword(const string& username, const string& newPassword) {
    string hashedPassword = hashPassword(newPassword);  // Băm mật khẩu mới
    string sql = "UPDATE users SET password = ? WHERE username = ?;";
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void UserDatabase::updateMustChangePassword(const string& username, int mustChangePassword) {
    string sql = "UPDATE users SET mustChangePassword = ? WHERE username = ?;";
    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, mustChangePassword);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void UserDatabase::updateUserInfo(const string& username, const string& fullName, const string& phoneNumber) {
	string sql = "UPDATE users SET fullName = ?, phoneNumber = ? WHERE username = ?;";
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, fullName.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, phoneNumber.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

void UserDatabase::backupDatabase(const string& backupPath) {
    const string dbPath = "data/users.db";
    filesystem::create_directories(filesystem::path(backupPath).parent_path());

    ifstream src(dbPath, ios::binary);
    ofstream dst(backupPath, ios::binary);

    if (!src.is_open() || !dst.is_open()) {
        cerr << "Error: Unable to open database file or backup file." << endl;
        return;
    }

    dst << src.rdbuf();

    src.close();
    dst.close();
}

void UserDatabase::startAutomaticBackup(const string& backupPath, int intervalSeconds) {
    thread([this, backupPath, intervalSeconds]() {
        while (true) {
            this_thread::sleep_for(chrono::seconds(intervalSeconds));
            backupDatabase(backupPath);
        }
        }).detach();
}

void UserDatabase::addPendingChange(const string& username, const string& newFullName, const string& newPhoneNumber) {
    string sql = "INSERT OR REPLACE INTO pending_changes (username, newFullName, newPhoneNumber, status) VALUES (?, ?, ?, 'pending');";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, newFullName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, newPhoneNumber.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error adding pending change: " << sqlite3_errmsg(db) << endl;
        }
        sqlite3_finalize(stmt);
    }
}

tuple<string, string> UserDatabase::getPendingChange(const string& username) {
    string sql = "SELECT newFullName, newPhoneNumber FROM pending_changes WHERE username = ? AND status = 'pending';";
    sqlite3_stmt* stmt;
    string pendingFullName, pendingPhone;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            pendingFullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            pendingPhone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }
        sqlite3_finalize(stmt);
    }
    return std::make_tuple(pendingFullName, pendingPhone);
}

bool UserDatabase::hasPendingChange(const string& username) {
    string sql = "SELECT COUNT(*) FROM pending_changes WHERE username = ? AND status = 'pending';";
    sqlite3_stmt* stmt;
    int count = 0;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count > 0;
}

void UserDatabase::confirmPendingChange(const string& username) {
    string sql = "SELECT newFullName, newPhoneNumber FROM pending_changes WHERE username = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            string newFullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            string newPhoneNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            updateUserInfo(username, newFullName, newPhoneNumber);
        }
        sqlite3_finalize(stmt);
    }
    sql = "UPDATE pending_changes SET status = 'confirmed' WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void UserDatabase::rejectPendingChange(const string& username) {
    string sql = "DELETE FROM pending_changes WHERE username = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}