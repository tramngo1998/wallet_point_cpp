#pragma once

#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

struct User {
    string username;
    string password;
    string role;
    string fullName;
    string phoneNumber;
    int mustChangePassword;
};

#endif
