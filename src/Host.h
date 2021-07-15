#ifndef HOST_H
#define HOST_H

#include <boost/asio.hpp>

/*
    Client:
    void begin_accepting_client();
    void begin_accepting_user();
*/

// Host is an API class for Server and Client, which implement their own methods(api)
class Host {
    public:
    enum State {
        reserved = 0,
        disconnected = 1, // S:disconnected with C; and vice versa
        connected = 2, // S:received C's req; C:received S's req
        destroyed = 3 // C/S is persistent, won't destroy itself, unless error in Construction
    };
    static const size_t default_bufsize;

    Host();

    virtual void run() = 0;
    virtual void destroy() = 0;
    virtual void process_client_req() = 0; // different behaviour based on C/S
    virtual void process_server_req() = 0; // different behaviour based on C/S

    protected:

    boost::asio::io_context ioctx;

    // decide to leave the ep to Client/Server
    // // ep for host connection
    // boost::asio::ip::tcp::endpoint host_ep; // mainsock
    // // ep for source/destination connection
    // boost::asio::ip::tcp::endpoint target_ep;

    // // req for Client communication
    // Protocol::Request req;
    // req buffer
    std::string receive_buf;
    std::string send_buf;

    // C/S state
    State state; // deprecated, the C/S's calling logic has already been a kind of "state machine"
    std::shared_ptr<boost::asio::ip::tcp::socket> mainsock; // with C/S
    std::shared_ptr<boost::asio::ip::tcp::socket> sock; // with destination(C) or source(S) 

    size_t session_count;
};

#endif