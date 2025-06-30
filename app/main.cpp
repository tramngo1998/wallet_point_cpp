#include "userManager.h"

int main() {

    UserManager userManager;
    int choice;
	
    do {
        cout << "\nCHUONG TRINH QUAN LY NGUOI DUNG" << endl;
        cout << "1. Dang ky" << endl;
        cout << "2. Dang nhap" << endl;
        cout << "3. Thoat" << endl;
        cout << "Chon: "; cin >> choice;

        switch (choice) {
        case 1:
            userManager.registerUser();
            break;
        case 2:
            userManager.loginUser();
            break;
        case 3:
            cout << "Thoat chuong trinh." << endl;
            break;
        default:
            cout << "Lua chon khong hop le!" << endl;
        }
    } while (choice != 3);
    return 0;
}
