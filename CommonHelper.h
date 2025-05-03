#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <string>
#include <vector>
#include <fstream>

void showFAQ(const std::string& filename);
int promptForInt(const std::string& promptMessage);
void writeString(std::ofstream& ofs, const std::string& str);
bool readString(std::ifstream& ifs, std::string& str);
std::string toLower(const std::string& str);
int levenshteinDistance(const std::string& s1, const std::string& s2);
std::vector<std::string> tokenize(const std::string& text);
std::string trim(const std::string& str);

#endif
