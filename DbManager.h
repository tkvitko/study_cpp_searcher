#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <string>
#include <pqxx/pqxx>
#include <iostream>


struct WordPresence {
    // Структура, описывающая наличие слова и его частоту на конкрертном ресурсе
    std::string word;
    std::string url;
    unsigned short frequency;
};

class DbManager {
    // Класс для взаимодействия с базой данных

public:
    DbManager();
    ~DbManager();
    // метод для добавления полной информации о слове в базу данных
    bool insertPresence(WordPresence presence);
    // метод для получения наличия слов по поисковому запросу
    std::vector<std::string> getSortedUrlsByWords(std::vector<std::string> words);

private:
    // параметры подключения к базе данных
    pqxx::connection* conn = nullptr;

    std::string host;
    int port;
    std::string dbName;
    std::string userName;
    std::string password;

    // метод для создания таблиц базы данных, если их ещё нет
    void createTables();
    // метод для перевода вектора строк в форматированную строку
    std::string getStringFromVector(std::vector<int> sourceVector);
    // метод для добавления нового слова в базу данных
    bool insertWord(std::string word);
    // метод для добавления нового ресурса в базу данных
    bool insertUrl(std::string url);
    // метод проверки, есть ли слово в базе данных
    bool checkWordExists(std::string word);
    // метод проверки, есть ли ресурс в базе данных
    bool checkUrlExists(std::string url);
    // метод для получения id слова
    int getWordId(std::string word);
    // метод для получения id слов
    std::vector<int> getWordsIds(std::vector<std::string> words);
    // метод для получения всех ресурсов, содержащих слово
    std::vector<int> getUrlsIdsByWord(std::string word);
    // метод для получения всех ресурсов, содержащих каждое запрошенное слово
    std::vector<int> getUrlsIdsByWords(std::vector<std::string> words);
};

#endif // DBMANAGER_H
