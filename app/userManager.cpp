#include "userManager.h"
#include <cstdlib>

#define SECRET_KEY "JBSWY3DPEHPK3PXP" //Có thể ẩn đi theo yêu cầu bảo mật

UserManager::UserManager() {
    userDatabase = make_unique<UserDatabase>();
    walletManager = make_unique<WalletManager>();
	transactionManager = make_unique<TransactionManager>();
	userDatabase->backupDatabase("data/backup/users.db");
}

UserManager::~UserManager() {
}

string UserManager::generatePassword() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int length = 8;

    random_device rd;                      
    mt19937 generator(rd());                
    uniform_int_distribution<int> dist(0, chars.size() - 1); 

    string password;
    for (int i = 0; i < length; ++i) {
        password += chars[dist(generator)]; 
    }

    return password;
}

void UserManager::generateOTP() {
    char otp[7] = { 0 };
    OTPData data;
    totp_new(&data, SECRET_KEY, hmac_algo_sha1, getCurrentTime, 6, 30);
    totp_now(&data, otp);
    cout << "Ma OTP cua ban la: " << otp << endl;
    getCurrentTime();
}

bool UserManager::verifyOTP(const string& userOTP) {
    char otp[7] = { 0 };
    OTPData data;
    totp_new(&data, SECRET_KEY, hmac_algo_sha1, getCurrentTime, 6, 30);
    totp_now(&data, otp);
    cout << "Xin moi nhap OTP: " << userOTP << endl;
    getCurrentTime();
    return (userOTP == otp);
}

void UserManager::registerUser() {
    system("cls");
    string username, password, fullName, phoneNumber;
	int amount = 1000;

    cout << "\nDANG KY NGUOI DUNG" << endl;
    cout << "Nhap ten nguoi dung: ";
    cin >> username;

    if (userDatabase->userExists(username)) {
        cout << "Loi: Ten nguoi dung da ton tai!" << endl;
        return;
    }

    cout << "Nhap mat khau: ";
    cin >> password;

    cout << "Nhap ho va ten: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, fullName);

    regex phonePattern("^0\\d{9}$");
    do {
        cout << "Nhap so dien thoai: ";
        getline(cin, phoneNumber);
        if (!regex_match(phoneNumber, phonePattern))
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai." << endl;
        else
            break;
    } while (true);

    userDatabase->addUser(username, password, "user", fullName, phoneNumber, 0);

    if (!walletManager->deductFromMaster(amount)) {
        cout << "Loi: So du vi tong khong du de cap diem cho nguoi dung moi." << endl;
        return;
    }

    string walletId = walletManager->createWallet(username, amount); 

    if (!walletId.empty()) {
        cout << "Dang ky thanh cong! Ban co ngay 1000 diem!" << endl;
    }
    else {
        cout << "Dang ky thanh cong! Nhung co loi khi tao vi." << endl;
    }
}

void UserManager::registerUserForOthers() {
    system("cls");
    string username, password = generatePassword(), fullName, phoneNumber;
	int amount = 1000;

    cout << "\nNHAN VIEN QUAN LY - TAO TAI KHOAN HO" << endl;
    cout << "Nhap ten nguoi dung moi: ";
    cin >> username;

    if (userDatabase->userExists(username)) {
        cout << "Loi: Ten nguoi dung da ton tai!" << endl;
        return;
    }

    cout << "Nhap ho va ten: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  
    getline(cin, fullName);

    regex phonePattern("^0\\d{9}$");
    do {
        cout << "Nhap so dien thoai: ";
        getline(cin, phoneNumber);
        if (!regex_match(phoneNumber, phonePattern))
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai." << endl;
        else
            break;
    } while (true);

    userDatabase->addUser(username, password, "user", fullName, phoneNumber, 1);

    if (!walletManager->deductFromMaster(amount)) {
        cout << "Loi: So du vi tong khong du de cap diem cho nguoi dung moi." << endl;
        return;
    }

    string walletId = walletManager->createWallet(username, amount);

    if (!walletId.empty()) {
        cout << "Ban da duoc cap 1000 diem. Wallet ID: " << walletId << endl;
    }
    else {
        cout << "Dang ky thanh cong! Nhung co loi khi tao vi." << endl;
    }

    cout << "Tao tai khoan thanh cong!\nMat khau: " << password << " (Bat buoc doi sau khi dang nhap)" << endl;
}

User UserManager::loginUser() {
    string username, password;
    cout << "Nhap ten nguoi dung: ";
    cin >> username;

    cout << "Nhap mat khau: ";
    cin >> password;

    if (!userDatabase->userExists(username)) {
        cout << "Nguoi dung khong ton tai!" << endl;
        return User{};  
    }

    User user = userDatabase->getUser(username);

    if (!userDatabase->verifyPassword(password, user.password)) {
        cout << "Mat khau sai!" << endl;
        return User{};
    }

    if (user.mustChangePassword) {
        cout << "Ban can doi mat khau ngay lap tuc!" << endl;
        changePassword(username);
        return User{};
    }

    if (userDatabase->hasPendingChange(username)) {
		
        system("cls");
        auto [pendingFullName, pendingPhone] = userDatabase->getPendingChange(username);
		cout << "Tai khoan cua ban co thay doi chua duoc xac nhan!" << endl;
        cout << "Thong tin thay doi:" << endl;
        cout << "Ho ten moi: " << pendingFullName << endl;
        cout << "So dien thoai moi: " << pendingPhone << endl;
		cout << "----------------------------" << endl;

        generateOTP();
        string pendingOTP;
        cout << "\nNhap ma OTP de xac nhan thay doi (Canh bao: sai OTP thi thong tin se khong duoc cap nhat): ";
        cin >> pendingOTP;
        if (verifyOTP(pendingOTP)) {
            userDatabase->confirmPendingChange(username);
            cout << "Thay doi da duoc xac nhan!" << endl;
            user = userDatabase->getUser(username);
        }
        else {
            cout << "Ma OTP khong dung! Thay doi bi huy. Vui long thu lai sau." << endl;
            userDatabase->rejectPendingChange(username);
            return User{};
        }
    }

    cout << "Dang nhap thanh cong!" << endl;

    if (user.role == "manager") {
        showManagerMenu(username);
    }
    else {
        showUserMenu(username);
    }

    return user;
}

void UserManager::transferFunds(const string& senderUsername) {
    string receiverWalletId;
    int amount;
    cout << "Nhap vi nguoi nhan diem: ";
    cin >> receiverWalletId;
    cout << "Nhap so luong diem: ";
    cin >> amount;

    generateOTP();
    string userOTP;
    cout << "Xin moi nhap ma OTP de xac nhan giao dich: ";
    cin >> userOTP;

    if (!verifyOTP(userOTP)) {
        cout << "OTP xac thuc khong thanh cong. Vui long thuc hien lai giao dich!" << endl;
        return;
    }

    string senderWalletId = walletManager->getWalletIdByUsername(senderUsername);
    if (senderWalletId.empty()) {
        cout << "Khong tim thay thong tin vi!" << endl;
    }
    else {
        bool success = walletManager->transferPoints(senderWalletId, receiverWalletId, amount);
        string senderDisplay = userDatabase->getUser(senderUsername).fullName;
        string receiverUsername = walletManager->getUsernameByWalletId(receiverWalletId);
        string receiverDisplay = receiverUsername.empty()
            ? "Unknown"
            : userDatabase->getUser(receiverUsername).fullName;

        cout << (success ? "Giao dich thanh cong!" : "Giao dich that bai!") << endl;
        transactionManager->recordTransaction(senderWalletId, senderDisplay,
            receiverWalletId, receiverDisplay,
            amount, "Chuyen diem",
            success ? "Thanh cong" : "That bai");
    }
    cout << "Press any key to continue..." << endl;
    cin.ignore();
    cin.get();
}

void UserManager::changePassword(const string& username) {
    string newPassword, userOTP;

    cout << "Nhap mat khau moi: ";
    cin >> newPassword;

    generateOTP();
    cout << "Nhap ma OTP de xac nhan: ";
    cin >> userOTP;

    if (verifyOTP(userOTP)) {

        userDatabase->updateUserPassword(username, newPassword);
        userDatabase->updateMustChangePassword(username, 0);

        cout << "Doi mat khau thanh cong!" << endl;
    }
    else {
        cout << "Ma OTP khong dung! Huy doi mat khau." << endl;
    }
}

bool UserManager::loadUserInfo(const string& username, User& user) {
    if (!userDatabase->userExists(username)) {
        return false;
    }
    user = userDatabase->getUser(username);
    return true;
}

void UserManager::updateUserInfo(const string& currentUsername, User& currentUser) {
    string targetUsername;
    if (currentUser.role == "manager") {
        cout << "Nhap ten nguoi dung can cap nhat (nhap '0' de cap nhat thong tin cua ban): ";
        cin >> targetUsername;
        if (targetUsername == "0") {
            targetUsername = currentUsername;
        }
    }
    else {
        targetUsername = currentUsername;
    }

    User targetUser = userDatabase->getUser(targetUsername);
    if (targetUser.username.empty()) {
        cout << "Nguoi dung khong ton tai!" << endl;
        return;
    }

    string newFullName, newPhoneNumber, userOTP;

    cout << "Nhap ho ten moi (nhap '0' neu khong cap nhat): ";
    getline(cin >> ws, newFullName);

    if (newFullName == "0") {
        newFullName = targetUser.fullName;
    }

    cout << "Nhap so dien thoai moi (nhap '0' neu khong cap nhat): ";
    getline(cin, newPhoneNumber);

    if (newPhoneNumber == "0") {
        newPhoneNumber = targetUser.phoneNumber;
    }

    else if (!newPhoneNumber.empty()) {
        regex phonePattern("^0\\d{9}$");
        while (!regex_match(newPhoneNumber, phonePattern)) {
            cout << "So dien thoai khong dung dinh dang! Vui long nhap lai (hoac nhan Enter de bo qua): ";
            getline(cin, newPhoneNumber);
            if (newPhoneNumber.empty())
                break;
        }
    }

    generateOTP();
    cout << "Nhap ma OTP de xac nhan: ";
    cin >> userOTP;

    if (verifyOTP(userOTP)) {
        if (currentUser.role == "manager" && targetUsername != currentUsername) {
            userDatabase->addPendingChange(targetUsername, newFullName, newPhoneNumber);
            cout << "Cap nhat thong tin thanh cong! Thay doi se co hieu luc sau khi xac nhan." << endl;
        }
        else {
            userDatabase->updateUserInfo(targetUsername, newFullName, newPhoneNumber);
            cout << "Cap nhat thong tin thanh cong!" << endl;
        }
    }
    else {
        cout << "Ma OTP khong dung! Huy cap nhat." << endl;
    }
}

void UserManager::showManagerMenu(const string& username) {
    User manager = userDatabase->getUser(username);

    if (manager.role != "manager") {
        return;
    }

    int choice;
    do {
        int balance = walletManager->getBalance("000000");
        cout << "\n=== MENU QUAN LY ===" << endl;
        cout << "Ho va ten: " << manager.fullName << endl;
        cout << "So dien thoai: " << manager.phoneNumber << endl;
        cout << "So du vi tong: " << balance << " diem" << endl;
        cout << "Vai tro: Quan tri vien" << endl;
        cout << "----------------------------" << endl;
        cout << "1. Tao tai khoan ho" << endl;
        cout << "2. Doi mat khau" << endl;
        cout << "3. Cap nhat thong tin nguoi dung" << endl;
        cout << "4. Dang xuat" << endl;
        cout << "Chon: ";
        cin >> choice;

        switch (choice) {
        case 1:
            registerUserForOthers();
            break;
        case 2:
            changePassword(manager.username);
            manager = userDatabase->getUser(username);
            break;
        case 3:
            updateUserInfo(username, manager);
            manager = userDatabase->getUser(username);
            break;
        case 4:
            cout << "Dang xuat thanh cong!" << endl;
			system("cls");
            return;
        default:
            cout << "Lua chon khong hop le!" << endl;
        }
    } while (choice != 4);
}

void UserManager::showUserMenu(const string& username) {
    User user = userDatabase->getUser(username);
    int choice;
    do {
        system("cls");
        int balance = walletManager->getBalanceByUsername(username);
	    string walletid = walletManager->getWalletIdByUsername(username);
        cout << "\n=== MENU NGUOI DUNG ===" << endl;
        cout << "Ho va ten: " << user.fullName << endl;
        cout << "So dien thoai: " << user.phoneNumber << endl;
		cout << "So tai khoan: " << walletid  << endl;
        cout << "So du: " << balance << " diem" << endl;
        cout << "Vai tro: " << user.role << endl;
        cout << "----------------------------" << endl;
        cout << "1. Doi mat khau" << endl;
        cout << "2. Thay doi thong tin" << endl;
        cout << "3. Xem lich su giao dich" << endl;
        cout << "4. Chuyen diem" << endl;
        cout << "5. Dang xuat" << endl;
        cout << "Chon: ";
        cin >> choice;

        if (cin.fail()) {  
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Lua chon khong hop le! Vui long nhap lai." << endl;
            continue;
        }

        switch (choice) {
        case 1:
            changePassword(user.username);
            user = userDatabase->getUser(username);  
            break;
        case 2:
            updateUserInfo(username, user);
            user = userDatabase->getUser(username);
            break;
        case 3: {
            string walletId = walletManager->getWalletIdByUsername(username);
            vector<string> history = transactionManager->getTransactionHistory(walletId);
            cout << "\n--- Lich su giao dich ---" << endl;
            for (const auto& record : history) {
                cout << record << endl;
            }
            cout << "Nhan phim bat ky de tiep tuc..." << endl;
            cin.ignore(); cin.get();
            break;
        }
        case 4: {
            transferFunds(user.username);
            break;
        }
        case 5:
            cout << "Dang xuat thanh cong!" << endl;
			system("cls");
            return;
        default:
            cout << "Lua chon khong hop le!" << endl;
        }
    } while (choice != 5);
}
