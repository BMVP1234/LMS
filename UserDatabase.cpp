#include "userDatabase.h"
#include "commonHelper.h"
#include <iostream>
#include <utility>
#include <fstream>

UserDatabase::UserInfo::UserInfo() : isAdmin(false) {}
UserDatabase::UserInfo::UserInfo(std::string pwd, bool admin)
    : password(std::move(pwd)), isAdmin(admin) {}
void UserDatabase::UserInfo::serialize(std::ofstream& ofs) const {
    writeString(ofs, password);
    ofs.write(reinterpret_cast<const char*>(&isAdmin), sizeof(isAdmin));
}
bool UserDatabase::UserInfo::deserialize(std::ifstream& ifs) {
    if (!readString(ifs, password)) return false;
    return ifs.read(reinterpret_cast<char*>(&isAdmin), sizeof(isAdmin)).good();
}
const std::string& UserDatabase::UserInfo::getPassword() const { return password; }
bool UserDatabase::UserInfo::getIsAdmin() const { return isAdmin; }
void UserDatabase::UserInfo::setPassword(const std::string& newPassword) { password = newPassword; }
void UserDatabase::UserInfo::setIsAdmin(bool admin) { isAdmin = admin; }

UserDatabase::UserDatabase(const std::string& filename) : filename(filename) {
    load();
}

bool UserDatabase::validateCredentials(const std::string& username,
                                       const std::string& password,
                                       bool& isAdminOut) const {
    auto it = userCache.find(username);
    if (it == userCache.end()) {
        return false;
    }
    const UserInfo& userInfo = it->second;
    if (userInfo.getPassword() == password) {
        isAdminOut = userInfo.getIsAdmin();
        return true;
    }
    return false;
}

bool UserDatabase::registerUser(const std::string& username,
                                const std::string& password,
                                bool isAdmin) {
    if (username.empty()) {
        std::cerr << "Registration failed: Username cannot be empty.\n";
        return false;
    }
    if (password.empty()) {
        std::cerr << "Registration failed: Password cannot be empty.\n";
        return false;
    }
    if (usernameExists(username)) {
        std::cerr << "Registration failed: Username already exists.\n";
        return false;
    }
    userCache.emplace(username, UserInfo(password, isAdmin));
    save();
    std::cout << "User registered successfully!\n";
    return true;
}

bool UserDatabase::removeAccount(const std::string& username) {
    auto it = userCache.find(username);
    if (it == userCache.end()) {
        std::cerr << "User not found.\n";
        return false;
    }
    userCache.erase(it);
    save();
    return true;
}

bool UserDatabase::usernameExists(const std::string& username) const {
    return (userCache.find(username) != userCache.end());
}

bool UserDatabase::updatePassword(const std::string& username, const std::string& newPassword) {
    if (newPassword.empty()) {
        std::cerr << "Password reset failed: New password cannot be empty.\n";
        return false;
    }
    auto it = userCache.find(username);
    if (it == userCache.end()) {
        std::cerr << "Password reset failed: User not found.\n";
        return false;
    }
    it->second.setPassword(newPassword);
    save();
    std::cout << "Password reset successfully!\n";
    return true;
}

bool UserDatabase::updateUser(const std::string& oldUsername,
                              const std::string& newUsername,
                              const std::string& newPassword,
                              std::optional<bool> newIsAdmin) {
    auto it = userCache.find(oldUsername);
    if (it == userCache.end()) {
        std::cerr << "Update failed: User not found.\n";
        return false;
    }
    if (!newUsername.empty() && newUsername != oldUsername && usernameExists(newUsername)) {
        std::cerr << "Username change failed: New username already exists.\n";
        return false;
    }
    UserInfo oldInfo = it->second;
    userCache.erase(it);
    std::string finalUsername = newUsername.empty() ? oldUsername : newUsername;
    std::string finalPassword = newPassword.empty() ? oldInfo.getPassword() : newPassword;
    bool finalIsAdmin = newIsAdmin.has_value() ? newIsAdmin.value() : oldInfo.getIsAdmin();
    userCache.emplace(finalUsername, UserInfo(finalPassword, finalIsAdmin));
    save();
    std::cout << "User details updated successfully.\n";
    return true;
}

bool UserDatabase::getUserInfo(const std::string& username, UserInfo& userInfoOut) const {
    auto it = userCache.find(username);
    if (it == userCache.end()) {
        return false;
    }
    userInfoOut = it->second;
    return true;
}

void UserDatabase::displayAllUsers() const {
    if (userCache.empty()) {
        std::cout << "No users found.\n";
        return;
    }
    const int usernameWidth = 30;
    const int roleWidth = 15;
    std::string usernameSeparator(usernameWidth, '-');
    std::string roleSeparator(roleWidth, '-');
    std::cout << "\n--- All Users ---\n";
    std::cout << std::left << std::setw(usernameWidth) << "Username"
              << std::left << std::setw(roleWidth) << "Role" << "\n";
    std::cout << std::left << std::setw(usernameWidth) << usernameSeparator
              << std::left << std::setw(roleWidth) << roleSeparator << "\n";
    for (const auto& [uname, info] : userCache) {
        std::string role = info.getIsAdmin() ? "Administrator" : "User";
        std::cout << std::left << std::setw(usernameWidth) << uname
                  << std::left << std::setw(roleWidth) << role << "\n";
    }
}

void UserDatabase::load() {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "No user database found. Starting with an empty database.\n";
        return;
    }
    userCache.clear();
    while (true) {
        std::string username;
        if (!readString(inputFile, username)) {
            break;
        }
        UserInfo userInfo;
        if (!userInfo.deserialize(inputFile)) {
            break;
        }
        userCache.emplace(std::move(username), std::move(userInfo));
    }
}

void UserDatabase::save() {
    std::ofstream outputFile(filename, std::ios::binary | std::ios::trunc);
    if (!outputFile) {
        std::cerr << "Error saving user database.\n";
        return;
    }
    for (const auto& [username, userInfo] : userCache) {
        writeString(outputFile, username);
        userInfo.serialize(outputFile);
    }
}
