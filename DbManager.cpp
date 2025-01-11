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

bool DbManager::insertWord(std::string word)
{
    try {
        pqxx::transaction<> tx{ *conn };
        tx.exec("insert into words (word)"
                "values ('" + tx.esc(word) + "');");
        tx.commit();
        return true;

    } catch(std::exception const& e) {
        return false;
    }
}

bool DbManager::insertUrl(std::string url)
{
    try {
        pqxx::transaction<> tx{ *conn };
        tx.exec("insert into urls (url)"
                "values ('" + tx.esc(url) + "');");
        tx.commit();
        return true;

    } catch(std::exception const& e) {
        return false;
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
        if (!checkWordExists(word)) {
            insertWord(word);
        }

        // если ресурса нет в таблице ресурсов, добавляем
        if (!checkUrlExists(url)) {
            insertUrl(url);
        }

        // добавляем новую частоту слова на ресурсе
        tx.exec("insert into frequencies (word, url, frequency)"
                "values ('" + tx.esc(word) + "', '" + tx.esc(url) + "', '" + tx.esc(std::to_string(frequency)) + "');");
        tx.commit();
        return true;

    } catch(std::exception const& e) {
        return false;
    }
}

bool DbManager::checkWordExists(std::string word)
{
    pqxx::work tx{ *conn };
    auto id = tx.query_value<int>("select id from words " "where word = '" + tx.esc(word) + "';");
    if (id) {
        return true;
    }
    return false;
}

bool DbManager::checkUrlExists(std::string url)
{
    pqxx::work tx{ *conn };
    auto id = tx.query_value<int>("select id from urls " "where url = '" + tx.esc(url) + "';");
    if (id) {
        return true;
    }
    return false;
}

int DbManager::getWordId(std::string word)
{
    pqxx::work tx{ *conn };
    int id = tx.query_value<int>("select id from words " "where word = '" + tx.esc(word) + "';");
    return id;
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
    for (auto& id : tx.query<int>("select id from frequencies " "where word_id = '" + tx.esc(wordIdStr) + "';")) {
        urlIds.push_back(id);
    }
    return urlIds;
}

std::vector<std::string> DbManager::getSortedUrlsByWords(std::vector<std::string> words)
{
    std::vector<int> url_ids = getUrlsIdsByWords(words);
    std::vector<int> word_ids = getWordsIds(words);

    std::vector<std::string> sortedUrls;
    pqxx::work tx{ *conn };
    for (auto& [url, freq] : tx.query<int>("select u.url, sum(f.frequency) sum_freq from frequencies f"
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

std::vector<int> DbManager::getUrlsIdsByWords(std::vector<std::string> words)
{
    // вектор векторов для хранения списков ресерсов для каждого слова
    std::vector<std::vector<int>> urlIdsBatches;
    for (std::string word : words) {
        std::vector<int> urlIds = getUrlsIdsByWord(word);
        urlIdsBatches.push_back(urlIds);
    }

    // вектор ресурсов, в которых есть все запрошенные слова
    std::vector<int> intersection;

    if (urlIdsBatches.size() == 1) {
        // если слово одно, список ресурсов уже готов
        intersection = urlIdsBatches[0];
    } else {
        // сортировка векторов
        for (std::vector<int> urlIdsBatche : urlIdsBatches ) {
            std::sort(urlIdsBatche.begin(), urlIdsBatche.end());
        }

        // берем пересечение первых двух векторов
        std::vector<int> v1 = urlIdsBatches.back();
        urlIdsBatches.pop_back();

        std::vector<int> v2 = urlIdsBatches.back();
        urlIdsBatches.pop_back();

        std::set_intersection(v1.begin(),v1.end(),
                              v2.begin(),v2.end(),
                              back_inserter(intersection));

        // пока в списке ещё есть вектора, вынимаем их и делаем пересечение с текущим результатом
        while (urlIdsBatches.size() != 0) {
            std::vector<int> vx = urlIdsBatches.back();
            urlIdsBatches.pop_back();
            std::set_intersection(vx.begin(),vx.end(),
                                  intersection.begin(),intersection.end(),
                                  back_inserter(intersection));
        }
    }
    // в итоге в intersection хранится список ресурсов, в которых нашлись все запрошенные слова
    return intersection;
}
