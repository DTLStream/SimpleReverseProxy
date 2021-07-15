#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <boost/asio.hpp>
#include "ProxyProto.h"

// Session is used to forward data AFTER both sockets have been connected in Server/Client
class Session:
    public std::enable_shared_from_this<Session> {
    public:
    
    static const size_t default_bufsize;
    
    enum State {
        reserverd = 0,
        connected = 1, // created
        destroyed = 2 // on error, or terminated, call the destroy function
    };

    Session(
        std::shared_ptr<boost::asio::ip::tcp::socket> s1,
        std::shared_ptr<boost::asio::ip::tcp::socket> s2,
        size_t sess_id = 0
    );

    // ~Session() = default;
    ~Session();

    void run();

    protected:
    void destroy();
    void main2sock();
    void sock2main();
    void on_read_mainsock();
    void on_read_sock();
    private:
    // state to record session state
    State sessionstate;
    std::shared_ptr<boost::asio::ip::tcp::socket> mainsock;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock;
    size_t mainsock_pending_bytes;
    size_t sock_pending_bytes;
    std::string mainsock_readbuffer;
    std::string sock_readbuffer;
    // keep write buffer
    std::string sock_writebuffer; // after mainsock_readbuffer filled
    std::string mainsock_writebuffer; // after sock_readbuffer filled
    // session_id is used for debugging
    size_t session_id;
    std::string session_id_disp;
};


// Useful Utilities
void ignore_sigpipe();
extern uint32_t keepalive;
extern uint32_t tcp_keepalive_time;
extern uint32_t tcp_keepalive_interval;
extern uint32_t tcp_keepalive_cnt;
void keep_alive(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

#endif