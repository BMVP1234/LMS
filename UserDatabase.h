#ifndef USERDATABASE_H
#define USERDATABASE_H

#include <string>
#include <unordered_map>
#include <optional>

class UserDatabase {
public:
    class UserInfo {
    public:
        UserInfo();
        UserInfo(std::string pwd, bool admin);
        void serialize(std::ofstream& ofs) const;
        bool deserialize(std::ifstream& ifs);
        const std::string& getPassword() const;
        bool getIsAdmin() const;
        void setPassword(const std::string& newPassword);
        void setIsAdmin(bool admin);
    private:
        std::string password;
        bool isAdmin;
    };
    explicit UserDatabase(const std::string& filename = "userDatabase.bin");
    bool validateCredentials(const std::string& username,
                             const std::string& password,
                             bool& isAdminOut) const;
    bool registerUser(const std::string& username,
                      const std::string& password,
                      bool isAdmin = false);
    bool removeAccount(const std::string& username);
    bool usernameExists(const std::string& username) const;
    bool updatePassword(const std::string& username, const std::string& newPassword);
    bool updateUser(const std::string& oldUsername,
                    const std::string& newUsername,
                    const std::string& newPassword,
                    std::optional<bool> newIsAdmin = std::nullopt);
    bool getUserInfo(const std::string& username, UserInfo& userInfoOut) const;
    void displayAllUsers() const;
private:
    void load();
    void save();
    std::string filename;
    std::unordered_map<std::string, UserInfo> userCache;
};

#endif
