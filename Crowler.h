#ifndef CROWLER_H
#define CROWLER_H

#include <string>
#include <regex>
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include "DbManager.h"
#include "SafeQueue.h""

class Crowler
{
private:

    // вектор для хранения потоков обработки задач
    std::vector<std::thread> threadsPool_;
    // очередь задач на обработку
    SafeQueue tasksQueue_;

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
    // обход ресурса
    void processUrl(std::string url, short depth);
    void work();

public:
    Crowler();
    ~Crowler();
    void addToCrowlingQueue(std::string url, unsigned short depth);
};

#endif // CROWLER_H
