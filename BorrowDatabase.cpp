#include "borrowDatabase.h"
#include "commonHelper.h"
#include <iostream>
#include <utility>

BorrowDatabase::BorrowRecord::BorrowRecord() : borrowDate(0) {}
BorrowDatabase::BorrowRecord::BorrowRecord(std::string usr, std::string bk, std::time_t date)
    : username(std::move(usr)), bookTitle(std::move(bk)), borrowDate(date) {}
void BorrowDatabase::BorrowRecord::serialize(std::ofstream& ofs) const {
    writeString(ofs, username);
    writeString(ofs, bookTitle);
    ofs.write(reinterpret_cast<const char*>(&borrowDate), sizeof(borrowDate));
}
bool BorrowDatabase::BorrowRecord::deserialize(std::ifstream& ifs) {
    if (!readString(ifs, username)) return false;
    if (!readString(ifs, bookTitle)) return false;
    if (!ifs.read(reinterpret_cast<char*>(&borrowDate), sizeof(borrowDate))) return false;
    return true;
}
const std::string& BorrowDatabase::BorrowRecord::getUsername() const { return username; }
const std::string& BorrowDatabase::BorrowRecord::getBookTitle() const { return bookTitle; }
std::time_t BorrowDatabase::BorrowRecord::getBorrowDate() const { return borrowDate; }

BorrowDatabase::BorrowDatabase(const std::string& f) : filename(f) {
    load();
}

BorrowDatabase::~BorrowDatabase() {
    save();
}

void BorrowDatabase::load() {
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "No borrow records found. Starting fresh.\n";
        return;
    }
    BorrowRecord record;
    while (record.deserialize(inputFile)) {
        borrowCache.emplace(record.getUsername(), record);
    }
    inputFile.close();
}

void BorrowDatabase::save() const {
    std::ofstream outputFile(filename, std::ios::binary | std::ios::trunc);
    if (!outputFile) {
        std::cerr << "Failed to save borrow records.\n";
        return;
    }
    for (const auto& [username, record] : borrowCache) {
        record.serialize(outputFile);
    }
    outputFile.close();
}

bool BorrowDatabase::borrowBook(const std::string& username, const std::string& bookTitle) {
    auto range = borrowCache.equal_range(username);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.getBookTitle() == bookTitle) {
            std::cerr << "You have already borrowed this book.\n";
            return false;
        }
    }
    BorrowRecord newRecord(username, bookTitle, std::time(nullptr));
    borrowCache.emplace(username, newRecord);
    save();
    return true;
}

bool BorrowDatabase::returnBook(const std::string& username, const std::string& bookTitle) {
    bool found = false;
    auto range = borrowCache.equal_range(username);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.getBookTitle() == bookTitle) {
            found = true;
            borrowCache.erase(it);
            save();
            break;
        }
    }
    if (!found) {
        std::cerr << "You have not borrowed this book.\n";
        return false;
    }
    return true;
}

std::vector<BorrowDatabase::BorrowRecord> BorrowDatabase::getBorrowedBooks(const std::string& username) const {
    std::vector<BorrowRecord> userBorrowed;
    auto range = borrowCache.equal_range(username);
    for (auto it = range.first; it != range.second; ++it) {
        userBorrowed.push_back(it->second);
    }
    return userBorrowed;
}
