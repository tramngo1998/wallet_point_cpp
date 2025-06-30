#ifndef USERDATABASE_H
#define USERDATABASE_H

#include <sqlite3.h>
#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>
#include <fstream>
#include <filesystem>
#include "user.h"

using namespace std;

class UserDatabase {
private:
    sqlite3* db;
    void initializeDatabase();

public:
    UserDatabase();
    ~UserDatabase();

    bool userExists(const string& username);
	User getUser(const string& username);
    void addUser(const string& username, const string& password, const string& role, const string& fullName, const string& phoneNumber, int mustChangePassword);
    void updateUserPassword(const string& username, const string& newPassword);
	void updateMustChangePassword(const string& username, int mustChangePassword);
    void updateUserInfo(const string& username, const string& fullName, const string& phoneNumber);

    string hashPassword(const string& password);
    bool verifyPassword(const string& inputPassword, const string& storedHashedPassword); 

    void backupDatabase(const string& backupPath);
    void startAutomaticBackup(const string& backupPath, int intervalSeconds);

    void addPendingChange(const string& username, const string& newFullName, const string& newPhoneNumber);
    tuple<string, string> getPendingChange(const string& username);
    bool hasPendingChange(const string& username);
    void confirmPendingChange(const string& username);
    void rejectPendingChange(const string& username);
};

#endif
