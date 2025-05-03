#ifndef MENU_H
#define MENU_H

#include <string>

class UserDatabase;
class BookDatabase;
class BorrowDatabase;

void forgotPassword(UserDatabase& userDB);
void manageAccounts(UserDatabase& userDB);
void handleManageBooks(BookDatabase& bookDB);
void handleAdminMenu(BookDatabase& bookDB, UserDatabase& userDB, BorrowDatabase& borrowDB);
void updateUser(UserDatabase& userDB, std::string& currentUsername);
void handleBorrowMenu(BookDatabase& bookDB, BorrowDatabase& borrowDB, const std::string& currentUsername);
void handleReturnBook(UserDatabase& userDB, BookDatabase& bookDB, BorrowDatabase& borrowDB, const std::string& currentUsername);
void handleReaderMenu(UserDatabase& userDB, std::string& currentUsername, BookDatabase& bookDB, BorrowDatabase& borrowDB);

#endif
