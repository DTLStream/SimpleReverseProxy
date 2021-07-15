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
    std::shared_ptr<boost::asio::ip::tcp::socket> s2,
    size_t sess_id
):sessionstate(connected),mainsock(s1),sock(s2),
    mainsock_pending_bytes(0),sock_pending_bytes(0),
    mainsock_readbuffer(default_bufsize,0),sock_readbuffer(default_bufsize,0),
    session_id(sess_id) {
    session_id_disp = "["+std::to_string(session_id)+"]";
    Log::setTitle("Session"+session_id_disp);
};

Session::~Session() {
    Log::setTitle("Session Destruction"+session_id_disp);
    Log::Log<Log::info>("end");
}

void Session::run() {
    main2sock();
    sock2main();
}

void Session::destroy() {
    Log::setTitle("Session::destroy"+session_id_disp);
    if (sessionstate==destroyed) {
        return;
    } else {
        Log::Log<Log::info>("destroy");
        if (sock) {
            sock->cancel();
            sock.reset();
        }
        if (mainsock) {
            mainsock->cancel();
            mainsock.reset();
        }
        sessionstate = destroyed;
    }
}

void Session::main2sock() {
    Log::setTitle("Session::main2sock"+session_id_disp);
    Log::Log<Log::info>("begin");
    if (!mainsock) {
        Log::Log<Log::warning>("null mainsock");
        destroy();
        return;
    }
    auto ptr = shared_from_this();
    mainsock->async_read_some(
        buffer(mainsock_readbuffer),
        [this, ptr](const system::error_code &error, size_t read_bytes_) {
            Log::setTitle("main2sock-read"+session_id_disp);
            if (!error) {
                mainsock_pending_bytes += read_bytes_;
                // update sock_writebuffer
                sock_writebuffer = mainsock_readbuffer.substr(0, read_bytes_);
                // method signature modified!
                on_read_mainsock();
            } else if (error==error::eof) { // peer shutdown writing, but we may still write
                // not going to read data from mainsock
                Log::Log<Log::info>(error.message());
                Log::Log<Log::debug>("eof, not reading mainsock, shutdown sock-write");
                system::error_code ec;
                if (sock&&sock->is_open()) {
                    sock->shutdown(socket_base::shutdown_send,ec);
                    if (ec) {
                        Log::Log<Log::info>(ec.message());
                        destroy();
                    }
                }
            } else {
                Log::Log<Log::warning>(error.message());
                destroy();
            }
        }
    );
}

void Session::sock2main() {
    Log::setTitle("Session::sock2main"+session_id_disp);
    Log::Log<Log::info>("begin");
    if (!sock) {
        Log::Log<Log::warning>("null sock");
        destroy();
        return;
    }
    auto ptr = shared_from_this();
    sock->async_read_some(
        buffer(sock_readbuffer),
        [this, ptr](const system::error_code &error, size_t read_bytes_) {
            Log::setTitle("sock2main-read"+session_id_disp);
            if (!error) {
                sock_pending_bytes += read_bytes_;
                // update mainsock_writebuffer
                mainsock_writebuffer = sock_readbuffer.substr(0,read_bytes_);
                // method signature modified!
                on_read_sock();
            } else if (error==error::eof) { // peer shutdown writing, but we may still write
                // not going to read data from sock
                Log::Log<Log::info>(error.message());
                Log::Log<Log::debug>("eof, not reading sock, shutdown mainsock-write");
                system::error_code ec;
                if (mainsock&&mainsock->is_open()) {
                    mainsock->shutdown(socket_base::shutdown_send,ec);
                    if (ec) {
                        Log::Log<Log::info>(ec.message());
                        destroy();
                    }
                }
            } else {
                Log::Log<Log::warning>(error.message());
                destroy();
            }
        }
    );
}

void Session::on_read_mainsock() {
    Log::setTitle("Session::on_read_mainsock"+session_id_disp);
    auto ptr = shared_from_this();
    sock->async_write_some(
        buffer(sock_writebuffer),
        [this, ptr](const system::error_code &error, size_t write_bytes_) {
            Log::setTitle("on_read_mainsock-write"+session_id_disp);
            if (!error) {
                mainsock_pending_bytes -= write_bytes_;
                if (mainsock_pending_bytes==0) {
                    Log::Log<Log::debug>("complete write");
                    main2sock();
                } else {
                    Log::Log<Log::debug>("imcomplete write");
                    // update sock_writebuffer
                    sock_writebuffer.erase(
                        sock_writebuffer.begin(),
                        sock_writebuffer.begin()+write_bytes_
                    );
                    assert(mainsock_pending_bytes==sock_writebuffer.length());
                    on_read_mainsock();
                }
            } else {
                Log::Log<Log::warning>(error.message());
                destroy();
            }
        }
    );
}

void Session::on_read_sock() {
    Log::setTitle("Session::on_read_sock"+session_id_disp);
    auto ptr = shared_from_this();
    mainsock->async_write_some(
        buffer(mainsock_writebuffer),
        [this, ptr](const system::error_code &error, size_t write_bytes_) {
            Log::setTitle("on_read_sock-write"+session_id_disp);
            if (!error) {
                sock_pending_bytes -= write_bytes_;
                if (sock_pending_bytes==0) {
                    Log::Log<Log::debug>("complete write");
                    sock2main();
                } else {
                    Log::Log<Log::debug>("imcomplete write");
                    // update mainsock_writebuffer
                    mainsock_writebuffer.erase(
                        mainsock_writebuffer.begin(),
                        mainsock_writebuffer.begin()+write_bytes_
                    );
                    assert(sock_pending_bytes==mainsock_writebuffer.length());
                    on_read_sock();
                }
            } else {
                Log::Log<Log::warning>(error.message());
                destroy();
            }
        }
    );
}

// ignore sigpipe utility
void ignore_sigpipe() {
    Log::Log<Log::info>("ignore sigpipe");
    signal(SIGPIPE,SIG_IGN);
}

// TCP KEEP ALIVE UTILITIES
uint32_t keepalive = 1;
uint32_t tcp_keepalive_time = 30; // in seconds
uint32_t tcp_keepalive_interval = 10;
uint32_t tcp_keepalive_cnt = 5; // 5 packets before drop
void keep_alive(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
    if (!sock||!sock->is_open()) return;
    Log::Log<Log::info>("TCP keep alive");
    int ret;
    ret = setsockopt(sock->native_handle(),
        SOL_SOCKET,SO_KEEPALIVE,&keepalive,sizeof(keepalive));
    if (ret) perror("SO_KEEPALIVE");

#ifdef __APPLE__
#define TCP_KEEPIDLE TCP_KEEPALIVE
    ret = setsockopt(sock->native_handle(),IPPROTO_TCP,TCP_KEEPIDLE,
        &tcp_keepalive_time,sizeof(tcp_keepalive_time));
    if (ret) perror("TCP_KEEPIDLE");
#undef TCP_KEEPIDLE
#endif

    ret = setsockopt(sock->native_handle(),IPPROTO_TCP,TCP_KEEPINTVL,
        &tcp_keepalive_interval, sizeof(tcp_keepalive_interval));
    if (ret) perror("TCP_KEEPINTVL");

    ret = setsockopt(sock->native_handle(),IPPROTO_TCP,TCP_KEEPCNT,
        &tcp_keepalive_cnt,sizeof(tcp_keepalive_cnt));
    if (ret) perror("TCP_KEEPCNT");

}
