#ifndef SERVER_H
#define SERVER_H

#include "ProxyProto.h"
#include "Host.h"
#include <boost/asio.hpp>

class Server:public Host {
    public:
    static const size_t backlog; // can remove const and set it yourself, default to 128
    Server(
        const std::string &cip, uint16_t cport,
        const std::string &uip, uint16_t uport
    );
    void run(); // polymorphism method

    protected:

    // ep and accpt for Client connection
    boost::asio::ip::tcp::endpoint client_ep;
    boost::asio::ip::tcp::acceptor client_accpt;
    // ep and accpt for proxy users' connection
    boost::asio::ip::tcp::endpoint user_ep;
    boost::asio::ip::tcp::acceptor user_accpt;

    void destroy(); // polymorphism method

    void begin_accepting_client(); // Server-only
    void begin_accepting_user(); // Server-only
    void process_client_req(); // polymorphism method, receive
    void process_server_req(); // polymorphism method, send

    private:
};

#endif
