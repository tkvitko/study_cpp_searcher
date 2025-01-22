#include "Searcher.h"

Searcher::Searcher() {}
Searcher::~Searcher() {}

std::vector<std::string> Searcher::processSearchRequest(std::vector<std::string> words)
{
    // получение ресурсов из базы данных для ответа на запрос

    std::vector<std::string> result;
    try {
        DbManager dbManager = DbManager();
        result = dbManager.getSortedUrlsByWords(words);
    } catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
    return result;
}
