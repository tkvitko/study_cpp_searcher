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

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


Crowler::Crowler() {}

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

        // Set SNI Hostname (many hosts need this to handshake successfully)
        // if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
        // {
        //     beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        //     throw beast::system_error{ec};
        // }

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

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
        // std::cout << res << std::endl;
        // std::cout << boost::beast::buffers_to_string(res.body().data()) << std::endl;

        // Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        // if(ec == net::error::eof)
        // {
        //     // Rationale:
        //     // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        //     ec = {};
        // }
        // if(ec)
        //     throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
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

std::vector<WordPresence> Crowler::calculatePresences(std::vector<std::string> words, std::string url)
{
    std::vector<WordPresence> wordsPresence;

    // наполнение map словами и частотами
    std::unordered_map<std::string, unsigned short> map;
    for (const auto& word : words) {
        ++map[word];
    }

    // преобразование map к вектору структур
    for (const auto& wordFrequency : map) {
        WordPresence presence = WordPresence{wordFrequency.first, url, wordFrequency.second};
        wordsPresence.push_back(presence);
    }

    return wordsPresence;
}
