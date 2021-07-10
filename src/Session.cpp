#include <sys/socket.h>
#include <sys/times.h>
#include <signal.h>
#include "Log.h"
#include "Session.h"

using namespace boost;
using namespace boost::asio;

// Session::

const size_t Session::default_bufsize = 8192;

Session::Session(
    std::shared_ptr<boost::asio::ip::tcp::socket> s1,
    std::shared_ptr<boost::asio::ip::tcp::socket> s2
):sessionstate(connected),mainsock(s1),sock(s2),
    mainsock_pending_bytes(0),sock_pending_bytes(0),
    mainsock_readbuffer(default_bufsize,0),sock_readbuffer(default_bufsize,0) {
    Log::setTitle("Session");
    Log::Log<Log::info>("ignore sigpipe and setsockopt keepalive");
    // first to ignore sigpipe
    signal(SIGPIPE,SIG_IGN);
    // second to set keepalive
    bool keepalive = true;
    setsockopt(mainsock->native_handle(),SOL_SOCKET,SO_KEEPALIVE,&keepalive,sizeof(keepalive));
    setsockopt(sock->native_handle(),SOL_SOCKET,SO_KEEPALIVE,&keepalive,sizeof(keepalive));
    // third to set rcv/sndtimeo
    timeval period;
    period.tv_sec = 5; // 5s per keepalive pkt
    period.tv_usec = 0;
    setsockopt(mainsock->native_handle(),SOL_SOCKET,SO_RCVTIMEO,&period,sizeof(period));
    setsockopt(mainsock->native_handle(),SOL_SOCKET,SO_SNDTIMEO,&period,sizeof(period));
    setsockopt(sock->native_handle(),SOL_SOCKET,SO_RCVTIMEO,&period,sizeof(period));
    setsockopt(sock->native_handle(),SOL_SOCKET,SO_SNDTIMEO,&period,sizeof(period));
};

Session::~Session() {
    Log::setTitle("Session Destruction");
    Log::Log<Log::info>("end");
}

void Session::run() {
    main2sock();
    sock2main();
}

void Session::destroy() {
    Log::setTitle("Session::destroy");
    if (sessionstate==destroyed) {
        return;
    } else {
        Log::Log<Log::info>("destroy");
        sock->cancel();
        sock->close();
        mainsock->cancel();
        mainsock->close();
        sessionstate = destroyed;
    }
}

void Session::main2sock() {
    Log::setTitle("Session::main2sock");
    Log::Log<Log::info>("begin");
    auto ptr = shared_from_this();
    mainsock->async_read_some(
        buffer(mainsock_readbuffer),
        [this, ptr](const system::error_code &error, size_t read_bytes_) {
            Log::setTitle("main2sock-read");
            if (!error) {
                mainsock_pending_bytes += read_bytes_;
                on_read_mainsock(mainsock_readbuffer.substr(0, read_bytes_));
            } else {
                Log::Log<Log::info>(error.message());
                destroy();
            }
        }
    );
}

void Session::sock2main() {
    Log::setTitle("Session::sock2main");
    Log::Log<Log::info>("begin");
    auto ptr = shared_from_this();
    sock->async_read_some(
        buffer(sock_readbuffer),
        [this, ptr](const system::error_code &error, size_t read_bytes_) {
            Log::setTitle("sock2main-read");
            if (!error) {
                sock_pending_bytes += read_bytes_;
                on_read_sock(sock_readbuffer.substr(0, read_bytes_));
            } else {
                Log::Log<Log::info>(error.message());
                destroy();
            }
        }
    );
}

void Session::on_read_mainsock(const std::string &towrite) {
    Log::setTitle("Session::on_read_mainsock");
    auto ptr = shared_from_this();
    sock->async_write_some(
        buffer(towrite),
        [this, ptr, towrite](const system::error_code &error, size_t write_bytes_) {
            Log::setTitle("on_read_mainsock-write");
            if (!error) {
                mainsock_pending_bytes -= write_bytes_;
                if (mainsock_pending_bytes==0) {
                    Log::Log<Log::info>("completely write");
                    main2sock();
                } else {
                    Log::Log<Log::info>("imcompletely write");
                    on_read_mainsock(towrite.substr(write_bytes_,mainsock_pending_bytes));
                }
            } else {
                Log::Log<Log::info>(error.message());
                destroy();
            }
        }
    );
}

void Session::on_read_sock(const std::string &towrite) {
    Log::setTitle("Session::on_read_sock");
    auto ptr = shared_from_this();
    mainsock->async_write_some(
        buffer(towrite),
        [this, ptr, towrite](const system::error_code &error, size_t write_bytes_) {
            Log::setTitle("on_read_sock-write");
            if (!error) {
                sock_pending_bytes -= write_bytes_;
                if (sock_pending_bytes==0) {
                    Log::Log<Log::info>("completely write");
                    sock2main();
                } else {
                    Log::Log<Log::info>("imcompletely write");
                    on_read_sock(towrite.substr(write_bytes_,sock_pending_bytes));
                }
            } else {
                Log::Log<Log::info>(error.message());
                destroy();
            }
        }
    );
}