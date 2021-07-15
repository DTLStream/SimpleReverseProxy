#ifndef CLIENT_H
#define CLIENT_H

#include "ProxyProto.h"
#include "Host.h"
#include <boost/asio.hpp>

class Client:public Host {
    public:

    Client(
        const std::string &sip, uint16_t sport, // server
        const std::string &tip, uint16_t tport // target
    );
    void run(); // polymorphism method

    protected:

    // ep for Server connection
    boost::asio::ip::tcp::endpoint server_ep;
    // ep for target connection
    boost::asio::ip::tcp::endpoint target_ep;

    void destroy(); // polymorphism method

    void begin_connecting_server(); // Client-only
    void begin_connecting_target(); // Client-only
    void process_server_req(); // polymorphism method, receive
    void process_client_req(); // polymorphism method, send
    // deprecated, init socket inline
    // void refresh_server_socket(); // Client-only
    // void refresh_target_socket(); // Client-only
    
    private:
};

#endif
