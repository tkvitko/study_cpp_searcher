//
// Copyright (c) 2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, small
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional/optional_io.hpp>>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include "Searcher.h""

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace my_program_state
{
std::size_t
request_count()
{
    static std::size_t count = 0;
    return ++count;
}

std::time_t
now()
{
    return std::time(0);
}
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
    start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    beast::flat_buffer buffer_{8192};

    // The request message.
    http::request<http::string_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{
                                socket_.get_executor(), std::chrono::seconds(60)};

    // Asynchronously receive a complete request message.
    void
    read_request()
    {
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec,
                   std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if(!ec)
                    self->process_request();
            });
    }

    // Determine what needs to be done with the request message.
    void
    process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);

        switch(request_.method())
        {
        case http::verb::get:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            create_response();
            break;

        case http::verb::post:
            response_.result(http::status::ok);
            response_.set(http::field::server, "Beast");
            create_response_on_post();
            break;

        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "Invalid request-method '"
                << std::string(request_.method_string())
                << "'";
            break;
        }

        write_response();
    }

    // Construct a response message based on the program state.
    void
    create_response()
    {
        if(request_.target() == "/count")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                <<  "<head><title>Request count</title></head>\n"
                <<  "<body>\n"
                <<  "<h1>Request count</h1>\n"
                <<  "<p>There have been "
                <<  my_program_state::request_count()
                <<  " requests so far.</p>\n"
                <<  "</body>\n"
                <<  "</html>\n";
        }
        else if(request_.target() == "/time")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                <<  "<html>\n"
                <<  "<head><title>Current time</title></head>\n"
                <<  "<body>\n"
                <<  "<h1>Current time</h1>\n"
                <<  "<p>The current time is "
                <<  my_program_state::now()
                <<  " seconds since the epoch.</p>\n"
                <<  "</body>\n"
                <<  "</html>\n";
        }
        else if(request_.target() == "/search")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<body>\n"
                << "<form action=\"/search\" method=\"post\">\n"
                << "<label for=\"fname\">Search request:</label><br>\n"
                << "<input type=\"text\" id=\"request\" name=\"request\" value=\"what?\"><br><br>\n"
                << "<input type=\"submit\" value=\"Submit\">\n"
                << "</form>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "File not found\r\n";
        }
    }

    // Construct a response message based on the program state.
    void
    create_response_on_post()
    {
        // функция формирования ответа со списком ресурсов

        // получение payload запроса вида request=word1+word2+...
        std::string body = request_.body();
        std::cout << body << std::endl;

        // удаление из строки приставки request=
        std::vector<std::string> wordsStr;
        boost::split(wordsStr, body, boost::is_any_of("="));
        wordsStr.erase(wordsStr.begin());
        for (auto& i : wordsStr) {
            std::cout << i << std::endl;
        }

        // составление вектора слов
        std::vector<std::string> words;
        boost::split(words, wordsStr[0], boost::is_any_of("+"));
        for (auto& i : words) {
            std::cout << i << std::endl;
        }

        // поиск ресурсов по словам
        Searcher searcher = Searcher();
        std::vector<std::string> urls;
        try {
            urls = searcher.processSearchRequest(words);
        } catch (std::exception const& e) {
            std::cout << e.what() << std::endl;
        }

        // преобразование списка ресурсов к HTML-формату
        std::string hrefsStr = getHrefListStringFromVector(urls);

        // формирование ответа
        response_.set(http::field::content_type, "text/html");
        beast::ostream(response_.body())
            << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<body>\n"
            << "<form action=\"/search\" method=\"post\">\n"
            << "<label for=\"fname\">Search request:</label><br>\n"
            << "<input type=\"text\" id=\"request\" name=\"request\" value=\"what?\"><br><br>\n"
            << "<input type=\"submit\" value=\"Submit\">\n"
            << "</form>\n"
            << "<p>\n"
            << hrefsStr
            << "</p>\n"
            << "</body>\n"
            << "</html>\n";
    }

    // Asynchronously transmit the response message.
    void
    write_response()
    {
        auto self = shared_from_this();

        response_.content_length(response_.body().size());

        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t)
            {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    // Check whether we have spent enough time on this connection.
    void
    check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec)
            {
                if(!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
    }

    std::string getHrefListStringFromVector(std::vector<std::string> urlsVector)
    {
        // преобразование вектора ресурсов к их перечислению в формате HTML

        std::string line = "";
        auto it = urlsVector.begin();
        std::string val = *it;
        line += "<a href=\">";
        line += (val);
        line += "\">";
        line += (val);
        line += "</a>";

        while(it != urlsVector.end() - 1)
        {
            ++it;
            line += "<br>";
            std::string val = *it;
            line += "<a href=\">";
            line += (val);
            line += "\">";
            line += (val);
            line += "</a>";
        }
        return line;
    }
};

// "Loop" forever accepting new connections.
void
http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
    acceptor.async_accept(socket,
                          [&](beast::error_code ec)
                          {
                              if(!ec)
                                  std::make_shared<http_connection>(std::move(socket))->start();
                              http_server(acceptor, socket);
                          });
}

int
main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if(argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);

        ioc.run();
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
