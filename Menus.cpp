#include "menu.h"
#include "commonHelper.h"
#include "userDatabase.h"
#include "bookDatabase.h"
#include "borrowDatabase.h"
#include <iostream>
#include <filesystem>
#include <optional>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <unordered_set>
#include <iomanip>
#include <memory>

enum class ManageAccountAction {
    ADD_USER = 1,
    ADD_ADMIN,
    REMOVE_USER,
    REMOVE_ADMIN,
    VIEW_ALL_USERS,
    EXIT
};

bool hasAtLeastOneAdmin(const UserDatabase& userDB) {
    std::ifstream inputFile("userDatabase.bin", std::ios::binary);
    if (!inputFile) {
        return false;
    }
    UserDatabase::UserInfo userInfo;
    std::string fileUsername;
    while (readString(inputFile, fileUsername) && userInfo.deserialize(inputFile)) {
        if (userInfo.getIsAdmin()) {
            return true;
        }
    }
    return false;
}

void manageAccounts(UserDatabase& userDB) {
    while (true) {
        std::cout << "\nManage Accounts Menu:\n"
                  << "1. Add Regular User Account\n"
                  << "2. Add Administrator Account\n"
                  << "3. Remove Regular User Account\n"
                  << "4. Remove Administrator Account\n"
                  << "5. View All Users\n"
                  << "6. Exit to Admin Menu\n";
        int choice = promptForInt("Choose an option: ");
        ManageAccountAction selectedAction = static_cast<ManageAccountAction>(choice);
        switch (selectedAction) {
            case ManageAccountAction::ADD_USER: {
                std::cout << "Enter username: ";
                std::string u;
                std::getline(std::cin, u);
                u = trim(u);
                std::cout << "Enter password: ";
                std::string p;
                std::getline(std::cin, p);
                p = trim(p);
                userDB.registerUser(u, p);
                break;
            }
            case ManageAccountAction::ADD_ADMIN: {
                std::cout << "Enter username: ";
                std::string u;
                std::getline(std::cin, u);
                u = trim(u);
                std::cout << "Enter password: ";
                std::string p;
                std::getline(std::cin, p);
                p = trim(p);
                userDB.registerUser(u, p, true);
                break;
            }
            case ManageAccountAction::REMOVE_USER: {
                std::cout << "Enter the username to remove: ";
                std::string username;
                std::getline(std::cin, username);
                username = trim(username);
                UserDatabase::UserInfo userInfo;
                if (userDB.getUserInfo(username, userInfo)) {
                    if (userInfo.getIsAdmin()) {
                        std::cerr << "Cannot remove an administrator using this option.\n";
                    } else {
                        if (userDB.removeAccount(username)) {
                            std::cout << "Regular user account removed successfully.\n";
                        }
                    }
                } else {
                    std::cerr << "User not found.\n";
                }
                break;
            }
            case ManageAccountAction::REMOVE_ADMIN: {
                std::cout << "Enter the administrator username to remove: ";
                std::string username;
                std::getline(std::cin, username);
                username = trim(username);
                UserDatabase::UserInfo userInfo;
                if (userDB.getUserInfo(username, userInfo)) {
                    if (!userInfo.getIsAdmin()) {
                        std::cerr << "The specified user is not an administrator.\n";
                    } else {
                        if (userDB.removeAccount(username)) {
                            bool hasOtherAdmin = hasAtLeastOneAdmin(userDB);
                            if (!hasOtherAdmin) {
                                std::cerr << "Cannot remove the last administrator.\n";
                                userDB.registerUser(username, userInfo.getPassword(), true);
                            } else {
                                std::cout << "Administrator account removed successfully.\n";
                            }
                        } else {
                            std::cerr << "Failed to remove administrator account.\n";
                        }
                    }
                } else {
                    std::cerr << "Administrator not found.\n";
                }
                break;
            }
            case ManageAccountAction::VIEW_ALL_USERS:
                userDB.displayAllUsers();
                break;
            case ManageAccountAction::EXIT:
                return;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}

enum class AdminMenuOption {
    MANAGE_BOOKS = 1,
    MANAGE_ACCOUNTS,
    ADMIN_FAQ,
    SIGN_OUT
};

enum class ManageBooksOption {
    ADD_BOOK = 1,
    REMOVE_BOOK,
    UPDATE_BOOK,
    VIEW_ALL_BOOKS,
    RETURN_TO_ADMIN_MENU
};

void handleManageBooks(BookDatabase& bookDB) {
    while (true) {
        std::cout << "\nManage Books Menu:\n"
                  << "1. Add Book\n"
                  << "2. Remove Book\n"
                  << "3. Update Book\n"
                  << "4. View All Books\n"
                  << "5. Return to Admin Menu\n";
        int choice = promptForInt("Choose an option: ");
        ManageBooksOption selectedOption = static_cast<ManageBooksOption>(choice);
        switch (selectedOption) {
            case ManageBooksOption::ADD_BOOK: {
                std::cout << "Enter book title: ";
                std::string title;
                std::getline(std::cin, title);
                title = trim(title);
                std::cout << "Enter genres: ";
                std::string genres;
                std::getline(std::cin, genres);
                genres = trim(genres);
                std::cout << "Enter author: ";
                std::string author;
                std::getline(std::cin, author);
                author = trim(author);
                int year = promptForInt("Enter published year: ");
                int copies = promptForInt("Enter available copies: ");
                bookDB.addBook(title, genres, author, year, copies);
                break;
            }
            case ManageBooksOption::REMOVE_BOOK: {
                std::cout << "Enter the title of the book to remove: ";
                std::string title;
                std::getline(std::cin, title);
                title = trim(title);
                if (!bookDB.removeBook(title)) {
                    std::cerr << "Failed to remove book. Book not found.\n";
                }
                break;
            }
            case ManageBooksOption::UPDATE_BOOK: {
                std::cout << "Enter the title of the book to update: ";
                std::string title;
                std::getline(std::cin, title);
                title = trim(title);
                std::cout << "Enter new title (blank = keep current): ";
                std::string newTitle;
                std::getline(std::cin, newTitle);
                newTitle = trim(newTitle);
                std::cout << "Enter new genres (blank = keep current): ";
                std::string newGenres;
                std::getline(std::cin, newGenres);
                newGenres = trim(newGenres);
                std::cout << "Enter new author (blank = keep current): ";
                std::string newAuthor;
                std::getline(std::cin, newAuthor);
                newAuthor = trim(newAuthor);
                std::cout << "Enter new published year (Enter = skip): ";
                std::string yearInputStr;
                std::getline(std::cin, yearInputStr);
                yearInputStr = trim(yearInputStr);
                std::optional<int> newYear;
                if (!yearInputStr.empty()) {
                    try {
                        newYear = std::stoi(yearInputStr);
                    } catch (...) {
                        std::cerr << "Invalid year. Skipping.\n";
                    }
                }
                std::cout << "Enter new copies (Enter = skip): ";
                std::string copiesInputStr;
                std::getline(std::cin, copiesInputStr);
                copiesInputStr = trim(copiesInputStr);
                std::optional<int> newCopies;
                if (!copiesInputStr.empty()) {
                    try {
                        newCopies = std::stoi(copiesInputStr);
                        if (newCopies.value() < 0) {
                            std::cerr << "Available copies cannot be negative. Skipping.\n";
                            newCopies.reset();
                        }
                    } catch (...) {
                        std::cerr << "Invalid copies input. Skipping.\n";
                    }
                }
                if (!bookDB.updateBook(title, newTitle, newGenres, newAuthor,
                                       newYear.value_or(0),
                                       newCopies.has_value() ? newCopies.value() : -1)) {
                    std::cerr << "Failed to update book.\n";
                }
                break;
            }
            case ManageBooksOption::VIEW_ALL_BOOKS:
                bookDB.displayBooks();
                break;
            case ManageBooksOption::RETURN_TO_ADMIN_MENU:
                return;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}

void handleAdminMenu(BookDatabase& bookDB, UserDatabase& userDB, BorrowDatabase& borrowDB) {
    while (true) {
        std::cout << "\nAdmin Menu:\n"
                  << "1. Manage Books\n"
                  << "2. Manage Accounts\n"
                  << "3. Admin FAQ\n"
                  << "4. Sign Out\n";
        int choice = promptForInt("Choose an option: ");
        AdminMenuOption option = static_cast<AdminMenuOption>(choice);
        switch (option) {
            case AdminMenuOption::MANAGE_BOOKS:
                handleManageBooks(bookDB);
                break;
            case AdminMenuOption::MANAGE_ACCOUNTS:
                manageAccounts(userDB);
                break;
            case AdminMenuOption::ADMIN_FAQ:
                showFAQ("AdminFAQ.txt");
                break;
            case AdminMenuOption::SIGN_OUT:
                std::cout << "Signing out...\n";
                return;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}

void forgotPassword(UserDatabase& userDB) {
    std::cout << "\nForgot Password\n";
    std::cout << "Enter your username: ";
    std::string username;
    std::getline(std::cin, username);
    username = trim(username);
    if (!userDB.usernameExists(username)) {
        std::cerr << "Username not found. Please try again.\n";
        return;
    }
    std::cout << "Enter your new password: ";
    std::string newPassword;
    std::getline(std::cin, newPassword);
    newPassword = trim(newPassword);
    if (userDB.updatePassword(username, newPassword)) {
        std::cout << "Password reset successfully!\n";
    } else {
        std::cerr << "Password reset failed. Please try again.\n";
    }
}

enum class UpdateUserOption {
    CHANGE_USERNAME = 1,
    CHANGE_PASSWORD,
    CHANGE_BOTH,
    RETURN
};

void updateUser(UserDatabase& userDB, std::string& currentUsername) {
    while (true) {
        std::cout << "\nUpdate User Menu:\n"
                  << "1. Change Username\n"
                  << "2. Change Password\n"
                  << "3. Change Both\n"
                  << "4. Return to Reader Menu\n";
        int choice = promptForInt("Choose an option: ");
        UpdateUserOption selectedOption = static_cast<UpdateUserOption>(choice);
        switch (selectedOption) {
            case UpdateUserOption::CHANGE_USERNAME: {
                std::cout << "Enter your current password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);
                bool isAdmin;
                if (userDB.validateCredentials(currentUsername, password, isAdmin)) {
                    std::cout << "Enter your new username: ";
                    std::string newUsername;
                    std::getline(std::cin, newUsername);
                    newUsername = trim(newUsername);
                    if (newUsername.empty()) {
                        std::cerr << "Username change failed: new username is empty.\n";
                        break;
                    }
                    if (userDB.updateUser(currentUsername, newUsername, "")) {
                        currentUsername = newUsername;
                        std::cout << "Username changed successfully.\n";
                    } else {
                        std::cerr << "Username change failed. Possibly already taken.\n";
                    }
                } else {
                    std::cerr << "Password confirmation failed.\n";
                }
                break;
            }
            case UpdateUserOption::CHANGE_PASSWORD: {
                std::cout << "Enter your current password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);
                bool isAdmin;
                if (userDB.validateCredentials(currentUsername, password, isAdmin)) {
                    std::cout << "Enter your new password: ";
                    std::string newPassword;
                    std::getline(std::cin, newPassword);
                    newPassword = trim(newPassword);
                    if (newPassword.empty()) {
                        std::cerr << "Password change failed: new password is empty.\n";
                        break;
                    }
                    if (userDB.updateUser(currentUsername, "", newPassword)) {
                        std::cout << "Password changed successfully.\n";
                    } else {
                        std::cerr << "Failed to update password.\n";
                    }
                } else {
                    std::cerr << "Password confirmation failed.\n";
                }
                break;
            }
            case UpdateUserOption::CHANGE_BOTH: {
                std::cout << "Enter your current password: ";
                std::string password;
                std::getline(std::cin, password);
                password = trim(password);
                bool isAdmin;
                if (userDB.validateCredentials(currentUsername, password, isAdmin)) {
                    std::cout << "Enter your new username (blank = keep current): ";
                    std::string newUsername;
                    std::getline(std::cin, newUsername);
                    newUsername = trim(newUsername);
                    std::cout << "Enter your new password (blank = keep current): ";
                    std::string newPassword;
                    std::getline(std::cin, newPassword);
                    newPassword = trim(newPassword);
                    if (newUsername.empty() && newPassword.empty()) {
                        std::cerr << "No changes provided.\n";
                        break;
                    }
                    if (userDB.updateUser(currentUsername, newUsername, newPassword)) {
                        if (!newUsername.empty()) {
                            currentUsername = newUsername;
                        }
                        std::cout << "Username and/or password changed successfully.\n";
                    } else {
                        std::cerr << "Change failed. Possibly new username is taken.\n";
                    }
                } else {
                    std::cerr << "Password confirmation failed.\n";
                }
                break;
            }
            case UpdateUserOption::RETURN:
                return;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}

void handleBorrowMenu(BookDatabase& bookDB, BorrowDatabase& borrowDB, const std::string& currentUsername);

void handleReturnBook(UserDatabase& userDB,
                      BookDatabase& bookDB,
                      BorrowDatabase& borrowDB,
                      const std::string& currentUsername);

enum class ReaderMenuOption {
    BORROW = 1,
    RETURN_BOOK,
    USER_PROFILE,
    VIEW_HISTORY,
    READER_FAQ,
    SIGN_OUT
};

void handleReaderMenu(UserDatabase& userDB,
                      std::string& currentUsername,
                      BookDatabase& bookDB,
                      BorrowDatabase& borrowDB)
{
    while (true) {
        std::cout << "\nReader Menu:\n"
                  << "1. Borrow\n"
                  << "2. Return Book\n"
                  << "3. User Profile\n"
                  << "4. View History\n"
                  << "5. Reader FAQ\n"
                  << "6. Sign Out\n";
        int choice = promptForInt("Choose an option: ");
        ReaderMenuOption selectedOption = static_cast<ReaderMenuOption>(choice);
        switch (selectedOption) {
            case ReaderMenuOption::BORROW:
                handleBorrowMenu(bookDB, borrowDB, currentUsername);
                break;
            case ReaderMenuOption::RETURN_BOOK:
                handleReturnBook(userDB, bookDB, borrowDB, currentUsername);
                break;
            case ReaderMenuOption::USER_PROFILE:
                updateUser(userDB, currentUsername);
                break;
            case ReaderMenuOption::VIEW_HISTORY: {
                auto borrowedBooks = borrowDB.getBorrowedBooks(currentUsername);
                if (borrowedBooks.empty()) {
                    std::cout << "You have no borrowed books.\n";
                } else {
                    std::cout << "\nYour Borrowed Books:\n";
                    const int titleWidth = 30;
                    const int dateWidth = 25;
                    std::string titleSeparator(titleWidth, '-');
                    std::string dateSeparator(dateWidth, '-');
                    std::cout << std::left << std::setw(titleWidth) << "Book Title"
                              << std::left << std::setw(dateWidth) << "Borrowed On" << "\n";
                    std::cout << std::left << std::setw(titleWidth) << titleSeparator
                              << std::left << std::setw(dateWidth) << dateSeparator << "\n";
                    for (const auto& record : borrowedBooks) {
                        std::time_t borrowDate = record.getBorrowDate();
                        std::string ctimeStr = std::ctime(&borrowDate);
                        if (!ctimeStr.empty() && ctimeStr.back() == '\n') {
                            ctimeStr.pop_back();
                        }
                        std::cout << std::left << std::setw(titleWidth) << record.getBookTitle()
                                  << std::left << std::setw(dateWidth) << ctimeStr << "\n";
                    }
                }
                break;
            }
            case ReaderMenuOption::READER_FAQ:
                showFAQ("ReaderFAQ.txt");
                break;
            case ReaderMenuOption::SIGN_OUT:
                std::cout << "Signing out...\n";
                return;
            default:
                std::cerr << "Invalid choice.\n";
        }
    }
}

void handleBorrow(BookDatabase& bookDB, BorrowDatabase& borrowDB, const std::string& currentUsername);

void handleBorrowMenu(BookDatabase& bookDB, BorrowDatabase& borrowDB, const std::string& currentUsername) {
    handleBorrow(bookDB, borrowDB, currentUsername);
}
