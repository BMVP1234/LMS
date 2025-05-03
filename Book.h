#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <fstream>

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

#endif // BOOK_H
