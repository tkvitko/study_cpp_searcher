#ifndef SEARCHER_H
#define SEARCHER_H

#include <string>
#include <vector>
#include "DbManager.h"

class Searcher
{
    // класс для обработки запросов поиска

public:
    Searcher();
    ~Searcher();
    // обработка поискового запроса и выдача отсортированного списка ресурсов в ответ
    std::vector<std::string> processSearchRequest(std::vector<std::string> words);
};

#endif // SEARCHER_H
