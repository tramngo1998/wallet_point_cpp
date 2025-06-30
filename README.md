# User Management & Wallet System 🚀

Một ứng dụng C++20 triển khai hệ thống quản lý người dùng với ví điện tử và các tính năng bảo mật dựa trên OTP. Hệ thống hỗ trợ đăng ký, đăng nhập, giao dịch ví và sao lưu cơ sở dữ liệu bằng SQLite.

## ✨ Tính năng chính

### 👤 Quản lý người dùng

* **Đăng ký:**
    * Đăng ký tiêu chuẩn cho người dùng mới.
    * Đăng ký do quản trị viên thực hiện (với mật khẩu tự động tạo).
* **Đăng nhập:**
    * Xác thực mật khẩu và **xác minh OTP**.
    * Xử lý các thay đổi hồ sơ **đang chờ xử lý** (pending) khi đăng nhập (yêu cầu xác nhận OTP).
* **Cập nhật hồ sơ:**
    * Người dùng tự cập nhật thông tin ngay lập tức.
    * Thay đổi do quản trị viên thực hiện yêu cầu người dùng xác nhận qua OTP (trạng thái **pending**).
* **Quản lý mật khẩu:**
    * Thay đổi mật khẩu yêu cầu **xác minh OTP**.
    * Buộc thay đổi mật khẩu đối với tài khoản được đánh dấu.

### 💼 Quản lý ví

* Số dư ví ban đầu khi đăng ký (ví dụ: 1000 điểm).
* Chuyển tiền giữa người dùng với **xác nhận OTP**.
* Lưu trữ và xem lịch sử giao dịch.

### 🔒 Bảo mật

* **Tạo & Xác minh OTP:** Sử dụng thuật toán `TOTP` (HMAC-SHA1 của OpenSSL).
* **Băm mật khẩu:** Sử dụng `SHA-256`.
* Mọi thao tác nhạy cảm đều yêu cầu **xác minh OTP**.

### 🗄️ Cơ sở dữ liệu & Sao lưu

* Lưu trữ dữ liệu người dùng, ví, và các thay đổi **đang chờ xử lý** trong `SQLite`.
* Tính năng sao lưu cơ sở dữ liệu tự động.

## 📂 Cấu trúc tệp quan trọng

```plaintext
app/
├── Header Files/
│   ├── cotp.h
│   ├── otp_utils.h         # Tiện ích OTP
│   ├── sqlite3.h           # Thư viện SQLite
│   ├── transactionManager.h # Quản lý giao dịch
│   ├── user.h              # Định nghĩa User
│   ├── userDatabase.h      # Tương tác DB
│   ├── userManager.h       # Quản lý User
│   └── walletManager.h     # Quản lý Ví
├── Source Files/
│   ├── cotp.cpp
│   ├── main.cpp            # Điểm vào ứng dụng
│   ├── otp_utils.cpp
│   ├── sqlite3.c
│   ├── transactionManager.cpp
│   ├── userDatabase.cpp
│   ├── userManager.cpp
│   └── walletManager.cpp

```

## 🛠️ Yêu cầu xây dựng

* Trình biên dịch `C++20` (Khuyến nghị Visual Studio).
* Thư viện `SQLite3`.
* Thư viện `OpenSSL`.
* Thư viện C++ chuẩn.

## ⚙️ Hướng dẫn xây dựng

1.  **Clone Repository:**
    ```bash
    git clone <your-repository-url>
    cd <repository-directory>
    ```
2.  **Cài đặt Dependencies:**
    * Đảm bảo `SQLite3` và `OpenSSL` đã được cài đặt.
    * Cấu hình đường dẫn thư viện và include trong môi trường/IDE của bạn (ví dụ: Visual Studio Project Properties).
3.  **Biên dịch dự án:**
    * Mở solution/project bằng IDE (Visual Studio, etc.) được cấu hình với `C++20`.
    * Build dự án (Compile & Link).

## 🚀 Hướng dẫn sử dụng

1.  **Chạy ứng dụng:** Thực thi file chạy đã biên dịch. Menu chính sẽ xuất hiện trên console.
2.  **Đăng ký người dùng:**
    * Người dùng tự đăng ký: Nhập username, password, full name, phone number.
    * Admin đăng ký hộ: Admin cung cấp thông tin, mật khẩu tự tạo, người dùng phải đổi mật khẩu khi đăng nhập lần đầu.
3.  **Đăng nhập & Xử lý Pending Changes:**
    * Nhập username và password.
    * Nếu có thay đổi **đang chờ xử lý** từ admin:
        * Hệ thống hiển thị thông tin mới (pending):
            ```
            New Full Name: [Tên mới đang chờ]
            New Phone Number: [Số điện thoại mới đang chờ]
            ```
        * Nhập OTP để **xác nhận** (áp dụng thay đổi) hoặc **từ chối** (hủy thay đổi).
    * Nếu không có pending changes hoặc đã xử lý xong, đăng nhập thành công sau khi xác minh OTP (nếu là lần đăng nhập chuẩn).
4.  **Ví & Giao dịch:** Truy cập menu ví để xem số dư, lịch sử, và thực hiện chuyển tiền (yêu cầu **OTP**).
5.  **Cập nhật Hồ sơ & Mật khẩu:**
    * Tự cập nhật: Thay đổi được áp dụng ngay.
    * Admin cập nhật hộ: Thay đổi chuyển sang trạng thái **pending**.
    * Đổi mật khẩu: Luôn yêu cầu **OTP**.

## 📝 Luồng công việc ví dụ

1.  **User A đăng ký:** Nhận 1000 điểm vào ví.
2.  **Admin cập nhật** tên và số điện thoại cho User A. Thay đổi này ở trạng thái **pending**.
3.  **User A đăng nhập:**
    * Nhập username/password.
    * Hệ thống báo có pending changes và hiển thị thông tin mới.
    * User A nhập OTP.
    * Nếu OTP đúng, hồ sơ được cập nhật. Nếu sai, thay đổi bị hủy.
    * User A tiếp tục vào menu chính.

## 📌 Ghi chú

* Khóa bí mật OTP (`OTP secret key`) hiện đang được định nghĩa trong `app/userManager.cpp`. Cân nhắc chuyển ra file cấu hình hoặc biến môi trường để bảo mật tốt hơn.
* Ứng dụng hỗ trợ song ngữ: **Tiếng Việt** và **Tiếng Anh**.
* Sao lưu cơ sở dữ liệu tự động giúp đảm bảo an toàn dữ liệu.
