#ifndef BORROWDATABASE_H
#define BORROWDATABASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <ctime>

class BorrowDatabase {
public:
    class BorrowRecord {
    public:
        BorrowRecord();
        BorrowRecord(std::string usr, std::string bk, std::time_t date);
        void serialize(std::ofstream& ofs) const;
        bool deserialize(std::ifstream& ifs);
        const std::string& getUsername() const;
        const std::string& getBookTitle() const;
        std::time_t getBorrowDate() const;
    private:
        std::string username;
        std::string bookTitle;
        std::time_t borrowDate;
    };
    explicit BorrowDatabase(const std::string& filename = "borrowedBooks.bin");
    ~BorrowDatabase();
    void load();
    void save() const;
    bool borrowBook(const std::string& username, const std::string& bookTitle);
    bool returnBook(const std::string& username, const std::string& bookTitle);
    std::vector<BorrowRecord> getBorrowedBooks(const std::string& username) const;
private:
    std::unordered_multimap<std::string, BorrowRecord> borrowCache;
    std::string filename;
};

#endif
