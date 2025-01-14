#ifndef CROWLER_H
#define CROWLER_H

#include <string>
#include <regex>
#include "DbManager.h"

class Crowler
{
private:
    // скачивание html по url
    std::string download(std::string url);
    // получение данных из HTML
    std::vector<std::string> getDataFromHtml(std::string s, std::regex filter);
    // получение слов из скачанного html
    std::vector<std::string> getWords(std::string innerHtml);
    // получение ссылок из скачанного html
    std::vector<std::string> getSubUrls(std::string innerHtml);
    // вычисление частот слов и сохранение данных в базу
    void savePresencesToDb(std::vector<std::string> words, std::string url);

public:
    Crowler();
    void processUrl(std::string url, short depth);
};

#endif // CROWLER_H
