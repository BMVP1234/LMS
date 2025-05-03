#include "Book.h"
#include "CommonHelper.h"
#include <stdexcept>

Book::Book() : publishedYear(0), availableCopies(0) {}

Book::Book(std::string title, std::string genres, std::string author,
           int publishedYear, int availableCopies)
    : title(std::move(title)),
      genres(std::move(genres)),
      author(std::move(author)),
      publishedYear(publishedYear),
      availableCopies(availableCopies)
{
    if (availableCopies < 0) {
        throw std::invalid_argument("Available copies cannot be negative.");
    }
}

const std::string& Book::getTitle() const {
    return title;
}

const std::string& Book::getGenres() const {
    return genres;
}

const std::string& Book::getAuthor() const {
    return author;
}

std::string Book::getPublishedYear() const {
    if (publishedYear < 0) {
        return std::to_string(-publishedYear) + " BCE";
    }
    return std::to_string(publishedYear);
}

int Book::getAvailableCopies() const {
    return availableCopies;
}

void Book::setAvailableCopies(int copies) {
    if (copies < 0) {
        throw std::invalid_argument("Available copies cannot be negative.");
    }
    availableCopies = copies;
}

void Book::setPublishedYear(int year) {
    publishedYear = year;
}

void Book::setTitle(std::string newTitle) {
    title = std::move(newTitle);
}

void Book::setGenres(std::string newGenres) {
    genres = std::move(newGenres);
}

void Book::setAuthor(std::string newAuthor) {
    author = std::move(newAuthor);
}

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
