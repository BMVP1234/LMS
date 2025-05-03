#ifndef BOOKDATABASE_H
#define BOOKDATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

class Book {
public:
    Book();
    Book(std::string title, std::string genres, std::string author,
         int publishedYear, int availableCopies);
    const std::string& getTitle() const;
    const std::string& getGenres() const;
    const std::string& getAuthor() const;
    std::string getPublishedYear() const;
    int getAvailableCopies() const;
    void setAvailableCopies(int copies);
    void setPublishedYear(int year);
    void setTitle(std::string newTitle);
    void setGenres(std::string newGenres);
    void setAuthor(std::string newAuthor);
    void serialize(std::ofstream& ofs) const;
    bool deserialize(std::ifstream& ifs);
private:
    std::string title;
    std::string genres;
    std::string author;
    int publishedYear;
    int availableCopies;
};

class BookDatabase {
public:
    BookDatabase(const std::string& bookFilename = "bookDatabase.bin",
                 const std::string& indexFilename = "tokenized_titles.bin");
    bool addBook(const std::string& title, const std::string& genres,
                 const std::string& author, int publishedYear, int availableCopies);
    bool removeBook(const std::string& title);
    bool updateBook(const std::string& title, const std::string& newTitle,
                    const std::string& newGenres, const std::string& newAuthor,
                    int newYear, int newCopies);
    void displayBooks() const;
    std::vector<Book*> searchByTitle(const std::string& query,
                                     const std::optional<std::string>& genresFilter,
                                     const std::optional<int>& publishedYearFilter,
                                     const std::optional<std::string>& authorFilter) const;
    bool updateAvailableCopies(const std::string& title, int newCopies);
    Book* getBookByTitle(const std::string& title) const;
private:
    void buildindexCache();
    void indexBook(Book* bookPtr);
    void removeBookFromIndex(const std::string& title);
    bool bookExists(const std::string& title) const;
    void saveindexCache() const;
    void loadindexCache();
    std::string bookFilename;
    std::string indexFilename;
    std::unordered_map<std::string, std::vector<Book*>> indexCache;
    std::vector<std::unique_ptr<Book>> booksCache;
};

#endif
