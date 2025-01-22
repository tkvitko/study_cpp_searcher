#include "Crowler.h"
#include "root_certificates.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "DbManager.h"
#include "IniParser.h"

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


Crowler::Crowler()
{
    const int cores_count = std::thread::hardware_concurrency();    // количество аппаратных ядер
    for (size_t i = 0; i != cores_count; ++i) {
        // наполнение вектора, хранящего потоки, задачами на обработку
        std::thread t(&Crowler::work, this);
        threadsPool_.push_back(std::move(t));
    }
}

Crowler::~Crowler()
{
    for (auto& thread : threadsPool_) {
        // ожидание окончания работы потоков
        thread.join();
    }
    threadsPool_.clear();
}

void Crowler::processUrl(std::string url, short depth)
{
    // полный процессинг ресурса: получение слов, сохранение а базу данных, получение внутренних ресурсов
    std::string html = download(url);
    std::vector<std::string> words = getWords(html);
    std::vector<std::string> subUrls = getSubUrls(html);
    savePresencesToDb(words, url);

    // если глубина не 1, обход внутренних ресурсов с уменьшенной на 1 глубиной
    // если рекурсивно (или сразу) попали сюда с глубиной 1, дальнейшего обхода не будет
    if (depth != 1) {
        depth--;
        for (auto& subUrl : subUrls) {
            processUrl(subUrl, depth);
        }
    }
}

std::string Crowler::download(std::string url)
{
    try
    {
        std::string const host = url;
        std::string const port = "443";
        std::string const target = "/";
        int const version = 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx(ssl::context::tlsv12_client);

        // This holds the root certificate used for verification
        load_root_certificates(ctx);

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_peer);

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        {
            boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            throw boost::system::system_error{ ec };
        }

        // Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        std::string const strBody = boost::beast::buffers_to_string(res.body().data());
        beast::error_code ec;
        stream.shutdown(ec);
        return strBody;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return "";
    }
}

std::vector<std::string> Crowler::getDataFromHtml(std::string s, std::regex filter)
{
    std::vector<std::string> result;

    auto words_begin = std::sregex_iterator(s.begin(), s.end(), filter);
    auto words_end = std::sregex_iterator();
    std::regex remove("<a href=");

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string match_str = std::regex_replace(match.str(), remove, "");
        result.push_back(match_str);
    }
    return result;
}

std::vector<std::string> Crowler::getWords(std::string s)
{
    std::regex word_regex("(?![^<]*?>)(\\w+)");
    return getDataFromHtml(s, word_regex);
}

std::vector<std::string> Crowler::getSubUrls(std::string s)
{
    std::regex word_regex("<a href=\"(.*?)\"");
    return getDataFromHtml(s, word_regex);
}

void Crowler::savePresencesToDb(std::vector<std::string> words, std::string url)
{
    std::vector<WordPresence> wordsPresence;

    // наполнение map словами и частотами
    std::unordered_map<std::string, unsigned short> map;
    for (const auto& word : words) {
        ++map[word];
    }

    // преобразование map к структуре и сохранение в базу данных
    DbManager dbManager = DbManager();
    for (const auto& wordFrequency : map) {
        WordPresence presence = WordPresence{wordFrequency.first, url, wordFrequency.second};
        dbManager.insertPresence(presence);
    }

}


void Crowler::processStartPage()
{
    IniParser parser(CONFIG_PATH);
    std::string url = parser.get_value<std::string>("Crowler.startPage");
    unsigned short depth = parser.get_value<unsigned short>("Crowler.recursionDepth");
    addToCrowlingQueue(url, depth);
}

void Crowler::addToCrowlingQueue(std::string url, unsigned short depth)
{
    UrlCrowlingTask task = {url, depth};
    tasksQueue_.push(task);
}

void Crowler::work() {
    while (true) {
        if (!tasksQueue_.isEmpty()) {
            // если в очереди задач есть задачи, вынимаем одну и выполняем
            UrlCrowlingTask task;
            tasksQueue_.pop(task);
            processUrl(task.url, task.depth);
        } else {
            // иначе передаем управление другому потоку
            std::this_thread::yield();
        }
    }
}
