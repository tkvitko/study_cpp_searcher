#ifndef CROWLER_H
#define CROWLER_H

#include <string>
#include <regex>
#include "DbManager.h"

class Crowler
{
private:
    // получение данных из HTML
    std::vector<std::string> getDataFromHtml(std::string s, std::regex filter);

public:
    Crowler();
    // скачивание html по url
    std::string download(std::string url);
    // получение слов из скачанного html
    std::vector<std::string> getWords(std::string innerHtml);
    // получение ссылок из скачанного html
    std::vector<std::string> getSubUrls(std::string innerHtml);
    // вычисление частот слов
    std::vector<WordPresence> calculatePresences(std::vector<std::string> words, std::string url);
};

#endif // CROWLER_H
