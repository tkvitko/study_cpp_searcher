#include <iostream>
#include <string>
#include <vector>
#include "Crowler.h"


int main()
{
    Crowler crowler = Crowler();
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
    // DbManager dbManager = DbManager();
    // std::string res = dbManager.getStringFromVector(test);
    // std::cout << res << std::endl;

    return 0;
}
