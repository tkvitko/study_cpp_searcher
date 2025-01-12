#include "DbManager.h"
#include <algorithm>



DbManager::DbManager()
{
    try {
        conn = new pqxx::connection("host=localhost "
                                    "port=5432 "
                                    "dbname=searcher "
                                    "user=searcher "
                                    "password=searcher");
    } catch (pqxx::sql_error e) {
        std::cout << e.what() << std::endl;
    }
}

DbManager::~DbManager()
{
    conn = nullptr;
    delete conn;
}

void DbManager::createTables()
{
    pqxx::transaction<> tx{ *conn };

    // таблица слов
    tx.exec("CREATE TABLE IF NOT EXISTS words ( "
            "id SERIAL primary key, "
            "word varchar(32) not null uniq "
            ");");

    // таблица ресурсов
    tx.exec("CREATE TABLE IF NOT exists urls ( "
            "id SERIAL primary key, "
            "url varchar(256) not null unique "
            ");");

    // таблица частот слов по ресурсам
    tx.exec("CREATE TABLE IF NOT exists frequencies ( "
            "id SERIAL primary key, "
            "word_id serial, "
            "url_id serial, "
            "frequency int not null, "

            "UNIQUE KEY (word_id, url_id)"
            "FOREIGN KEY (word_id) REFERENCES words (id), "
            "FOREIGN KEY (url_id) REFERENCES urls (id) "
            ");");

    tx.commit();
}

std::string DbManager::getStringFromVector(std::vector<int> sourceVector)
{
    std::string line = "";
    auto it = sourceVector.begin();
    int val = *it;
    line += std::to_string(val);

    while(it != sourceVector.end() - 1)
    {
        ++it;
        line += ',';
        int val = *it;
        line += std::to_string(val);
    }
    return line;
}

template <typename T>
std::vector<std::pair<T, std::size_t>> DbManager::adjacent_count(const std::vector<T>& v)
// https://stackoverflow.com/questions/39596336/how-to-count-equal-adjacent-elements-in-a-vector
{
    std::vector<std::pair<T, std::size_t>> res;

    for (auto it = v.begin(), e = v.end(); it != e; /*Empty*/) {
        auto it2 = std::adjacent_find(it, e, std::not_equal_to<>{});
        if (it2 != e) {
            ++it2;
        }
        res.emplace_back(*it, std::distance(it, it2));
        it = it2;
    }
    return res;
}

unsigned int DbManager::insertWord(std::string word)
{
    try {
        pqxx::transaction<> tx{ *conn };
        pqxx::result r = tx.exec("insert into words (word)"
                "values ('" + tx.esc(word) + "') returning id;");
        tx.commit();
        return r.inserted_oid();

    } catch(std::exception const& e) {
        return 0;
    }
}

unsigned int DbManager::insertUrl(std::string url)
{
    try {
        pqxx::transaction<> tx{ *conn };
        pqxx::result r = tx.exec("insert into urls (url)"
                "values ('" + tx.esc(url) + "') RETURNING id;");
        tx.commit();
        return r.inserted_oid();

    } catch(std::exception const& e) {
        return 0;
    }
}

bool DbManager::insertPresence(WordPresence presence)
{
    try {
        pqxx::transaction<> tx{ *conn };
        std::string word = presence.word;
        std::string url = presence.url;
        unsigned short frequency = presence.frequency;

        // если слова нет в таблице слов, добавляем
        unsigned int wordId = getWordId(word);
        if (wordId == 0) {
            wordId = insertWord(word); // здесь почему-то всегда 0, потому получаем отдельным запросом ниже
            wordId = getWordId(word);
        }

        // если ресурса нет в таблице ресурсов, добавляем
        unsigned int urlId = getUrlId(url);
        if (urlId == 0) {
            urlId = insertWord(url); // здесь почему-то всегда 0, потому получаем отдельным запросом ниже
            urlId = getUrlId(url);
        }

        std::string wordIdStr = std::to_string(wordId);
        std::string urlIdStr = std::to_string(urlId);

        // добавляем новую частоту слова на ресурсе
        tx.exec("insert into frequencies (word_id, url_id, frequency)"
                "values ('" + tx.esc(wordIdStr) + "', '" + tx.esc(urlIdStr) + "', '" + tx.esc(std::to_string(frequency)) + "') RETURNING id;");
        tx.commit();
        return true;

    } catch(std::exception const& e) {
        return false;
    }
}

unsigned int DbManager::getWordId(std::string word)
{
    try {
        pqxx::work tx{ *conn };
        unsigned int id = tx.query_value<unsigned int>("select id from words " "where word = '" + tx.esc(word) + "';");
        return id;
    } catch (std::exception const& ) {
        return 0;
    }
}

unsigned int DbManager::getUrlId(std::string url)
{
    try {
        pqxx::work tx{ *conn };
        unsigned int id = tx.query_value<unsigned int>("select id from urls " "where url = '" + tx.esc(url) + "';");
        return id;
    } catch (std::exception const& ) {
        return 0;
    }
}

std::vector<int> DbManager::getWordsIds(std::vector<std::string> words)
{
    std::vector<int> ids;
    for (auto& word : words) {
        ids.push_back(getWordId(word));
    }
    return ids;
}

std::vector<int> DbManager::getUrlsIdsByWord(std::string word)
{
    std::vector<int> urlIds;
    int wordId = getWordId(word);
    std::string wordIdStr = std::to_string(wordId);

    pqxx::work tx{ *conn };
    for (auto [id] : tx.query<int>("select id from frequencies " "where word_id = '" + tx.esc(wordIdStr) + "';")) {
        urlIds.push_back(id);
    }
    return urlIds;
}

std::vector<int> DbManager::getUrlsIdsByWords(std::vector<std::string> words)
{
    std::vector<int> urlIds;
    std::vector<int> urlIdsAccepted;
    std::vector<int> word_ids = getWordsIds(words);
    pqxx::work tx{ *conn };
    for (auto [urlIdd] : tx.query<int>("select url_id from frequencies " "where word_id in '" + getStringFromVector(word_ids) + "';")) {
        urlIds.push_back(urlIdd);
    }

    for (auto pair : adjacent_count(urlIds)) {
        int urlId = pair.first;
        int count = pair.second;
        if (words.size() == count) {
            urlIdsAccepted.push_back(urlId);
        }
    }
    return urlIdsAccepted;
}

std::vector<std::string> DbManager::getSortedUrlsByWords(std::vector<std::string> words)
{
    std::vector<int> url_ids = getUrlsIdsByWords(words);
    std::vector<int> word_ids = getWordsIds(words);

    std::vector<std::string> sortedUrls;
    pqxx::work tx{ *conn };
    for (auto& [url, freq] : tx.query<std::string, int>("select u.url, sum(f.frequency) sum_freq from frequencies f"
                                                        "join words w on f.word_id = w.id"
                                                        "join urls u on f.url_id = u.id"
                                                        "where word_id in (" + getStringFromVector(word_ids) + ")"
                                                        "and url_id in (" + getStringFromVector(url_ids) + ")"
                                                        "group by url"
                                                        "order by sum_freq DESC")) {
        sortedUrls.push_back(url);
    }
    return sortedUrls;
}
