#include "commonHelper.h"
#include "userDatabase.h"
#include "bookDatabase.h"
#include "borrowDatabase.h"
#include "menu.h"
#include <iostream>

enum class MainMenuOption {
    LOGIN = 1,
    REGISTER,
    FORGOT_PASSWORD,
    FAQ,
    EXIT
    EASTER_EGG
};

int main() {
    UserDatabase userDB("userDatabase.bin");
    BookDatabase bookDB("bookDatabase.bin", "tokenized_titles.bin");
    BorrowDatabase borrowDB("borrowedBooks.bin");
    bool isAdmin;
    if (!userDB.validateCredentials("Administrator", "Quinella", isAdmin) || !isAdmin) {
        if (userDB.registerUser("Administrator", "Quinella", true)) {
            std::cout << "Administrator account created.\n";
        } else {
            std::cerr << "Failed to create Administrator account.\n";
        }
    }
    while (true) {
        std::cout << "\nWelcome to the Library Management System\n"
                  << "1. Login\n"
                  << "2. Register\n"
                  << "3. Forgot Password\n"
                  << "4. FAQ\n"
                  << "5. Exit\n";
        int choice = promptForInt("Choose an option: ");
        switch (static_cast<MainMenuOption>(choice)) {
            case MainMenuOption::LOGIN: {
                std::cout << "Enter username: ";
                std::string username;
                std::getline(std::cin, username);
                username = trim(username);
                std::cout << "Enter password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);
                bool adm;
                if (userDB.validateCredentials(username, password, adm)) {
                    if (adm) {
                        std::cout << "Welcome, Administrator.\n";
                        handleAdminMenu(bookDB, userDB, borrowDB);
                    } else {
                        std::cout << "Login successful! Welcome, " << username << "!\n";
                        handleReaderMenu(userDB, username, bookDB, borrowDB);
                    }
                } else {
                    std::cerr << "Invalid credentials. Please try again.\n";
                }
                break;
            }
            case MainMenuOption::REGISTER: {
                std::cout << "Choose a username: ";
                std::string un;
                std::getline(std::cin, un);
                un = trim(un);
                std::cout << "Choose a password: ";
                std::string pw;
                std::getline(std::cin, pw);
                pw = trim(pw);
                userDB.registerUser(un, pw);
                break;
            }
            case MainMenuOption::FORGOT_PASSWORD:
                forgotPassword(userDB);
                break;
            case MainMenuOption::FAQ:
                showFAQ("mainFAQ.txt");
                break;
            case MainMenuOption::EXIT:
                std::cout << "Goodbye! Exiting the system.\n";
                break;
            case MainMenuOption::EASTER_EGG
                std::cout << "Just Monika" ;
                return 0;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}
