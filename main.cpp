#include <iostream>
#include <string>
#include <vector>
#include "Crowler.h"


int main()
{
    // Crowler crowler = Crowler();
    // std::string res = crowler.download("litres.ru");
    // std::cout << res << std::endl;
    // std::vector<std::string> words = crowler.getWords(res);
    // for (auto& i : words) {
    //     std::cout << i << std::endl;
    // }

    // std::string test = "<html>\n<head><title>301 Moved Moved Moved Permanently</title></head>\n<a href=\"yandex.ru\">yandex</a><a href=\"yandex2.ru\">yandex</a></html>";
    // std::vector<std::string> words = crowler.getWords(test);
    // for (auto& i : words) {
    //     std::cout << i << std::endl;
    // }
    // std::vector<WordPresence> pres = crowler.calculatePresences(words, "test.url");
    // for (auto& i : pres) {
    //     std::cout << i.frequency << std::endl;
    // }

    // std::vector<int> test = {1, 2, 3, 4, 5};
    DbManager dbManager = DbManager();
    // unsigned int id = dbManager.insertWord("какжетак6");
    // std::cout << id << std::endl;

    // dbManager.insertWord("привет");
    // dbManager.insertWord("мир");
    // unsigned int id = dbManager.insertWord("пока3");
    // std::cout << id << std::endl;
    // dbManager.insertUrl("www.yandex.ru");
    // dbManager.insertUrl("www.google.com");
    // dbManager.insertUrl("www.yahoo.com");

    // unsigned int id = dbManager.getWordId("привет");
    // std::cout << id << std::endl;

    std::string url = "www.yandex3.ru";
    std::string word = "новое3";
    unsigned short frequency = 2;
    WordPresence presence = {word, url, frequency};
    dbManager.insertPresence(presence);

    return 0;
}
