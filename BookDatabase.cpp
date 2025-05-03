#include "bookDatabase.h"
#include "commonHelper.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

Book::Book() : publishedYear(0), availableCopies(0) {}
Book::Book(std::string t, std::string g, std::string a, int y, int c)
    : title(std::move(t)), genres(std::move(g)), author(std::move(a)),
      publishedYear(y), availableCopies(c) {
    if (c < 0) {
        throw std::invalid_argument("Available copies cannot be negative.");
    }
}
const std::string& Book::getTitle() const { return title; }
const std::string& Book::getGenres() const { return genres; }
const std::string& Book::getAuthor() const { return author; }
std::string Book::getPublishedYear() const {
    return (publishedYear < 0)
        ? std::to_string(-publishedYear) + " BCE"
        : std::to_string(publishedYear);
}
int Book::getAvailableCopies() const { return availableCopies; }
void Book::setAvailableCopies(int copies) {
    if (copies < 0) {
        throw std::invalid_argument("Available copies cannot be negative.");
    }
    availableCopies = copies;
}
void Book::setPublishedYear(int year) { publishedYear = year; }
void Book::setTitle(std::string newTitle) { title = std::move(newTitle); }
void Book::setGenres(std::string newGenres) { genres = std::move(newGenres); }
void Book::setAuthor(std::string newAuthor) { author = std::move(newAuthor); }
void Book::serialize(std::ofstream& ofs) const {
    writeString(ofs, title);
    writeString(ofs, genres);
    writeString(ofs, author);
    ofs.write(reinterpret_cast<const char*>(&publishedYear), sizeof(publishedYear));
    ofs.write(reinterpret_cast<const char*>(&availableCopies), sizeof(availableCopies));
}
bool Book::deserialize(std::ifstream& ifs) {
    if (!readString(ifs, title)) return false;
    if (!readString(ifs, genres)) return false;
    if (!readString(ifs, author)) return false;
    if (!ifs.read(reinterpret_cast<char*>(&publishedYear), sizeof(publishedYear))) return false;
    if (!ifs.read(reinterpret_cast<char*>(&availableCopies), sizeof(availableCopies))) return false;
    return true;
}

BookDatabase::BookDatabase(const std::string& bf, const std::string& ix)
    : bookFilename(bf), indexFilename(ix) {
    loadindexCache();
}

bool BookDatabase::addBook(const std::string& title, const std::string& genres,
                           const std::string& author, int publishedYear, int availableCopies) {
    if (title.empty() || genres.empty() || author.empty() || availableCopies < 0) {
        std::cerr << "Invalid book details.\n";
        return false;
    }
    if (bookExists(title)) {
        std::cerr << "Book with the same title already exists.\n";
        return false;
    }
    std::ofstream outputFile(bookFilename, std::ios::binary | std::ios::app);
    if (!outputFile) {
        std::cerr << "Error opening book database for writing.\n";
        return false;
    }
    auto bookPtr = std::make_unique<Book>(title, genres, author, publishedYear, availableCopies);
    bookPtr->serialize(outputFile);
    outputFile.close();
    Book* rawPtr = bookPtr.get();
    booksCache.push_back(std::move(bookPtr));
    indexBook(rawPtr);
    saveindexCache();
    std::cout << "Book added successfully.\n";
    return true;
}

bool BookDatabase::removeBook(const std::string& title) {
    std::ifstream inputFile(bookFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Book database file not found.\n";
        return false;
    }
    std::ofstream tempFile("tempBookDatabase.bin", std::ios::binary | std::ios::trunc);
    if (!tempFile) {
        std::cerr << "Error creating temporary book database file.\n";
        return false;
    }
    Book book;
    bool found = false;
    while (book.deserialize(inputFile)) {
        if (book.getTitle() != title) {
            book.serialize(tempFile);
        } else {
            found = true;
            removeBookFromIndex(title);
        }
    }
    inputFile.close();
    tempFile.close();
    if (found) {
        std::filesystem::remove(bookFilename);
        std::filesystem::rename("tempBookDatabase.bin", bookFilename);
        std::cout << "Book removed successfully.\n";
        auto it = std::remove_if(booksCache.begin(), booksCache.end(),
            [title](const std::unique_ptr<Book>& b){
                return b->getTitle() == title;
            }
        );
        if (it != booksCache.end()) {
            booksCache.erase(it, booksCache.end());
        }
        saveindexCache();
        return true;
    } else {
        std::cerr << "Book not found.\n";
        std::filesystem::remove("tempBookDatabase.bin");
        return false;
    }
}

bool BookDatabase::updateBook(const std::string& title, const std::string& newTitle,
                              const std::string& newGenres, const std::string& newAuthor,
                              int newYear, int newCopies) {
    std::ifstream inputFile(bookFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Book database file not found.\n";
        return false;
    }
    std::ofstream tempFile("tempBookDatabase.bin", std::ios::binary | std::ios::trunc);
    if (!tempFile) {
        std::cerr << "Error creating temporary book database file.\n";
        return false;
    }
    Book book;
    bool found = false;
    while (book.deserialize(inputFile)) {
        if (book.getTitle() == title) {
            found = true;
            if (!newTitle.empty() && newTitle != title && bookExists(newTitle)) {
                std::cerr << "Book update failed: New title already exists.\n";
                std::filesystem::remove("tempBookDatabase.bin");
                return false;
            }
            if (!newTitle.empty()) {
                removeBookFromIndex(title);
            }
            if (!newTitle.empty()) book.setTitle(newTitle);
            if (!newGenres.empty()) book.setGenres(newGenres);
            if (!newAuthor.empty()) book.setAuthor(newAuthor);
            if (newYear != 0) book.setPublishedYear(newYear);
            if (newCopies >= 0) book.setAvailableCopies(newCopies);
            book.serialize(tempFile);
            if (!newTitle.empty()) {
                indexBook(&book);
            }
        } else {
            book.serialize(tempFile);
        }
    }
    inputFile.close();
    tempFile.close();
    if (found) {
        std::filesystem::remove(bookFilename);
        std::filesystem::rename("tempBookDatabase.bin", bookFilename);
        std::cout << "Book updated successfully.\n";
        for (auto& bookPtr : booksCache) {
            if (bookPtr->getTitle() == title) {
                if (!newTitle.empty()) {
                    bookPtr->setTitle(newTitle);
                    indexBook(bookPtr.get());
                }
                if (!newGenres.empty()) {
                    bookPtr->setGenres(newGenres);
                }
                if (!newAuthor.empty()) {
                    bookPtr->setAuthor(newAuthor);
                }
                if (newYear != 0) {
                    bookPtr->setPublishedYear(newYear);
                }
                if (newCopies >= 0) {
                    bookPtr->setAvailableCopies(newCopies);
                }
                break;
            }
        }
        saveindexCache();
        return true;
    } else {
        std::cerr << "Book not found.\n";
        std::filesystem::remove("tempBookDatabase.bin");
        return false;
    }
}

void BookDatabase::displayBooks() const {
    if (booksCache.empty()) {
        std::cout << "No books available in the library.\n";
        return;
    }
    const int titleWidth = 70;
    const int genresWidth = 15;
    const int authorWidth = 20;
    const int yearWidth = 8;
    const int copiesWidth = 5;
    std::string titleSeparator(titleWidth, '-');
    std::string genresSeparator(genresWidth, '-');
    std::string authorSeparator(authorWidth, '-');
    std::string yearSeparator(yearWidth, '-');
    std::string copiesSeparator(copiesWidth, '-');
    std::cout << "\n--- All Books ---\n";
    std::cout << std::left << std::setw(titleWidth) << "Title"
              << std::left << std::setw(genresWidth) << "Genres"
              << std::left << std::setw(authorWidth) << "Author"
              << std::left << std::setw(yearWidth) << "Year"
              << std::left << std::setw(copiesWidth) << "Copies" << "\n";
    std::cout << std::left << std::setw(titleWidth) << titleSeparator
              << std::left << std::setw(genresWidth) << genresSeparator
              << std::left << std::setw(authorWidth) << authorSeparator
              << std::left << std::setw(yearWidth) << yearSeparator
              << std::left << std::setw(copiesWidth) << copiesSeparator << "\n";
    for (const auto& bookPtr : booksCache) {
        std::cout << std::left << std::setw(titleWidth) << bookPtr->getTitle()
                  << std::left << std::setw(genresWidth) << bookPtr->getGenres()
                  << std::left << std::setw(authorWidth) << bookPtr->getAuthor()
                  << std::left << std::setw(yearWidth) << bookPtr->getPublishedYear()
                  << std::left << std::setw(copiesWidth) << bookPtr->getAvailableCopies() << "\n";
    }
}

std::vector<Book*> BookDatabase::searchByTitle(const std::string& query,
    const std::optional<std::string>& genresFilter,
    const std::optional<int>& publishedYearFilter,
    const std::optional<std::string>& authorFilter) const
{
    std::vector<Book*> results;
    if (query.empty()) {
        return results;
    }
    std::vector<std::string> keywords = tokenize(query);
    if (keywords.empty()) {
        return results;
    }
    const int threshold = 2;
    std::unordered_set<Book*> candidateBooks;
    for (const auto& keyword : keywords) {
        for (const auto& [token, books] : indexCache) {
            if (levenshteinDistance(keyword, token) <= threshold) {
                for (auto* book : books) {
                    candidateBooks.insert(book);
                }
            }
        }
    }
    std::unordered_map<Book*, int> bookMatchCount;
    for (auto* book : candidateBooks) {
        std::vector<std::string> titleTokens = tokenize(book->getTitle());
        for (const auto& keyword : keywords) {
            for (const auto& token : titleTokens) {
                if (levenshteinDistance(keyword, token) <= threshold) {
                    bookMatchCount[book]++;
                    break;
                }
            }
        }
    }
    std::vector<std::pair<Book*, int>> sortedBooks(bookMatchCount.begin(), bookMatchCount.end());
    std::sort(sortedBooks.begin(), sortedBooks.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    for (const auto& [bk, count] : sortedBooks) {
        if (genresFilter.has_value()) {
            std::string genresLower = toLower(genresFilter.value());
            std::string bookGenresLower = toLower(bk->getGenres());
            if (bookGenresLower.find(genresLower) == std::string::npos) {
                continue;
            }
        }
        if (publishedYearFilter.has_value()) {
            std::string yearStr = bk->getPublishedYear();
            std::string numericYear;
            for (char c : yearStr) {
                if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
                    numericYear += c;
                } else {
                    break;
                }
            }
            int bookYear = std::stoi(numericYear);
            if (bookYear != publishedYearFilter.value()) {
                continue;
            }
        }
        if (authorFilter.has_value()) {
            std::string authorLower = toLower(authorFilter.value());
            std::string bookAuthorLower = toLower(bk->getAuthor());
            if (bookAuthorLower.find(authorLower) == std::string::npos) {
                continue;
            }
        }
        results.push_back(bk);
    }
    return results;
}

bool BookDatabase::updateAvailableCopies(const std::string& title, int newCopies) {
    for (auto& bookPtr : booksCache) {
        if (bookPtr->getTitle() == title) {
            if (newCopies < 0) {
                std::cerr << "Available copies cannot be negative.\n";
                return false;
            }
            bookPtr->setAvailableCopies(newCopies);
            saveindexCache();
            return true;
        }
    }
    std::cerr << "Book not found.\n";
    return false;
}

Book* BookDatabase::getBookByTitle(const std::string& title) const {
    for (auto& bookPtr : booksCache) {
        if (bookPtr->getTitle() == title) {
            return bookPtr.get();
        }
    }
    return nullptr;
}

void BookDatabase::buildindexCache() {
    std::ifstream inputFile(bookFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Book database file not found. Starting with an empty database.\n";
        return;
    }
    Book book;
    size_t count = 0;
    while (book.deserialize(inputFile)) {
        auto bookPtr = std::make_unique<Book>(book);
        Book* rawPtr = bookPtr.get();
        booksCache.push_back(std::move(bookPtr));
        indexBook(rawPtr);
        count++;
    }
    inputFile.close();
    std::cout << "Loaded and indexed " << count << " books.\n";
}

void BookDatabase::indexBook(Book* bookPtr) {
    std::vector<std::string> tokens = tokenize(bookPtr->getTitle());
    for (const auto& token : tokens) {
        indexCache[token].push_back(bookPtr);
    }
}

void BookDatabase::removeBookFromIndex(const std::string& title) {
    std::vector<std::string> tokens = tokenize(title);
    for (const auto& token : tokens) {
        auto it = indexCache.find(token);
        if (it != indexCache.end()) {
            auto& books = it->second;
            books.erase(std::remove_if(books.begin(), books.end(),
                [&](Book* b){ return b->getTitle() == title; }),
                books.end());
            if (books.empty()) {
                indexCache.erase(it);
            }
        }
    }
}

bool BookDatabase::bookExists(const std::string& title) const {
    for (auto& b : booksCache) {
        if (b->getTitle() == title) {
            return true;
        }
    }
    return false;
}

void BookDatabase::saveindexCache() const {
    std::ofstream ofs(indexFilename, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        std::cerr << "Failed to open inverted index file for writing.\n";
        return;
    }
    size_t mapSize = indexCache.size();
    ofs.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    for (const auto& [token, books] : indexCache) {
        writeString(ofs, token);
        size_t bookCount = books.size();
        ofs.write(reinterpret_cast<const char*>(&bookCount), sizeof(bookCount));
        for (auto* b : books) {
            writeString(ofs, b->getTitle());
        }
    }
}

void BookDatabase::loadindexCache() {
    std::ifstream inputFile(bookFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Book database file not found. Starting with an empty database.\n";
        return;
    }
    Book book;
    size_t count = 0;
    while (book.deserialize(inputFile)) {
        auto bookPtr = std::make_unique<Book>(book);
        booksCache.push_back(std::move(bookPtr));
        count++;
    }
    inputFile.close();
    std::cout << "Loaded " << count << " books from " << bookFilename << ".\n";
    std::ifstream ifs(indexFilename, std::ios::binary);
    if (!ifs) {
        std::cerr << "Inverted index file not found. Building from book database.\n";
        buildindexCache();
        saveindexCache();
        return;
    }
    size_t mapSize;
    if (!ifs.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize))) {
        std::cerr << "Failed to read inverted index size. Rebuilding.\n";
        buildindexCache();
        saveindexCache();
        return;
    }
    for (size_t i = 0; i < mapSize; ++i) {
        std::string token;
        if (!readString(ifs, token)) {
            std::cerr << "Failed to read token. Rebuilding.\n";
            buildindexCache();
            saveindexCache();
            return;
        }
        size_t bookCount;
        if (!ifs.read(reinterpret_cast<char*>(&bookCount), sizeof(bookCount))) {
            std::cerr << "Failed to read book count. Rebuilding.\n";
            buildindexCache();
            saveindexCache();
            return;
        }
        for (size_t j = 0; j < bookCount; ++j) {
            std::string bookTitle;
            if (!readString(ifs, bookTitle)) {
                std::cerr << "Failed to read book title. Rebuilding.\n";
                buildindexCache();
                saveindexCache();
                return;
            }
            Book* bookPtr = nullptr;
            for (auto& b : booksCache) {
                if (b->getTitle() == bookTitle) {
                    bookPtr = b.get();
                    break;
                }
            }
            if (bookPtr) {
                indexCache[token].push_back(bookPtr);
            }
        }
    }
    std::cout << "Inverted index loaded successfully.\n";
}
