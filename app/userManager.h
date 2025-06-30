#pragma once

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <iostream>
#include <string>
#include <regex>
#include <map>
#include <memory>
#include <cotp.h>

#include "otp_utils.h"
#include "userDatabase.h"
#include "walletManager.h"
#include "transactionManager.h"
#include "user.h"

using namespace std;

class UserManager {
private:
    unique_ptr<UserDatabase> userDatabase;
    unique_ptr<WalletManager> walletManager;
    unique_ptr<TransactionManager> transactionManager;
	string generatePassword();

public:
    UserManager();
    ~UserManager();

    bool verifyOTP(const string& userOTP);
    void generateOTP();

    void registerUser();
    void registerUserForOthers();
    User loginUser();
    bool loadUserInfo(const string& username, User& user);
    void transferFunds(const string& senderUsername);
    void showManagerMenu(const string& username);
    void showUserMenu(const string& user);
    void changePassword(const string& username);
    void updateUserInfo(const string& username, User& user); 
};

#endif
