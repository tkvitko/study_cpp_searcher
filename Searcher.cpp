#include "Searcher.h"

Searcher::Searcher() {}
Searcher::~Searcher() {}

std::vector<std::string> Searcher::processSearchRequest(std::vector<std::string> words)
{
    // получение ресурсов из базы данных для ответа на запрос

    DbManager dbManager = DbManager();
    std::vector<std::string> result = dbManager.getSortedUrlsByWords(words);
    return result;
}
