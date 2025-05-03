#include "commonHelper.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

void showFAQ(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "FAQ file not found " << filename << "\n";
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        std::cout << line << "\n";
    }
}

int promptForInt(const std::string& promptMessage) {
    while (true) {
        if (!promptMessage.empty()) {
            std::cout << promptMessage;
        }
        std::string input;
        std::getline(std::cin, input);
        try {
            return std::stoi(input);
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid input. Please enter a valid number.\n";
        } catch (const std::out_of_range&) {
            std::cerr << "Number is out of range. Please try again.\n";
        }
    }
}

void writeString(std::ofstream& ofs, const std::string& str) {
    size_t size = str.size();
    ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
    ofs.write(str.data(), size);
}

bool readString(std::ifstream& ifs, std::string& str) {
    size_t size;
    if (!ifs.read(reinterpret_cast<char*>(&size), sizeof(size))) {
        return false;
    }
    str.resize(size);
    return ifs.read(&str[0], size).good();
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

int levenshteinDistance(const std::string& s1, const std::string& s2) {
    size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1+1, std::vector<int>(len2+1));
    for (size_t i = 0; i <= len1; ++i) d[i][0] = i;
    for (size_t j = 0; j <= len2; ++j) d[0][j] = j;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            d[i][j] = std::min({
                d[i-1][j] + 1,
                d[i][j-1] + 1,
                d[i-1][j-1] + (s1[i-1] == s2[j-1] ? 0 : 1)
            });
        }
    }
    return d[len1][len2];
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string currentToken;
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            currentToken += std::tolower(static_cast<unsigned char>(c));
        } else if (!currentToken.empty()) {
            tokens.push_back(currentToken);
            currentToken.clear();
        }
    }
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    return tokens;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}
