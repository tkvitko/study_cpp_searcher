#ifndef CROWLER_H
#define CROWLER_H

#include <string>
#include <regex>
#include <vector>
#include <thread>
#include "SafeQueue.h"

class Crowler
{
private:

    // вектор для хранения потоков обработки задач
    std::vector<std::thread> threadsPool_;
    // очередь задач на обработку
    SafeQueue tasksQueue_;

    // скачивание html по url
    std::string download(std::string domain, std::string path);
    // получение данных из HTML
    std::vector<std::string> getDataFromHtml(std::string s, std::regex filter);
    // получение слов из скачанного html
    std::vector<std::string> getWords(std::string innerHtml);
    // получение ссылок из скачанного html
    std::vector<std::string> getSubUrls(std::string innerHtml);
    // вычисление частот слов и сохранение данных в базу
    void savePresencesToDb(std::vector<std::string> words, std::string url);
    // обход ресурса
    void processUrl(std::string domain, std::string path, short depth);
    // добавление задачи в очередь на обход
    void addToCrowlingQueue(std::string domain, std::string pat, unsigned short depth);
    // методя для взятия очередной задачи на процессинг ресурса из очереди задач и процессинга
    void work();

public:
    Crowler();
    ~Crowler();
    // метод запуска процессинга стартового ресурса (из конфига)
    void processStartPage();
};

#endif // CROWLER_H
