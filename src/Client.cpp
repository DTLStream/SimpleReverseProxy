#include "Client.h"
#include "Log.h"
#include "Session.h"
using namespace boost;
using namespace boost::asio;

Client::Client(
        const std::string &sip, uint16_t sport, // server
        const std::string &tip, uint16_t tport // target
    ):Host(),
    server_ep(ip::address::from_string(sip),sport),
    target_ep(ip::address::from_string(tip),tport) {
}

void Client::run() {
    Log::setTitle("Client::run");
    Log::Log<Log::debug>("run client");
    begin_connecting_server();
    Log::Log<Log::warning>("ioctx.run");
    ioctx.run();
    Log::Log<Log::warning>("Client stopped");
}

void Client::destroy() {
    Log::setTitle("Client::destroy");
    if (state==destroyed) {
        Log::Log<Log::info>("already destroyed");
    } else {
        Log::Log<Log::info>("Client destroy");
        system::error_code error;
        Log::Log<Log::info>("stop mainsock");
        if (mainsock&&mainsock->is_open()) {
            mainsock->cancel(error);
            if (error) {
                Log::Log<Log::warning>(error.message());
            }
            mainsock->close(error);
            if (error) {
                Log::Log<Log::warning>(error.message());
            }
            mainsock.reset();
        }
        Log::Log<Log::info>("stop sock");
        if (sock&&sock->is_open()) {
            sock->cancel(error);
            if (error) {
                Log::Log<Log::warning>(error.message());
            }
            sock->close(error);
            if (error) {
                Log::Log<Log::warning>(error.message());
            }
        }
        state = destroyed;
    }
}

void Client::begin_connecting_server() {
    Log::setTitle("Client::begin_connecting_server");
    Log::Log<Log::warning>("begin connecting server");
    mainsock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
    // keep_alive(mainsock);
    mainsock->async_connect(
        server_ep,
        [this](const system::error_code &ec) {
            Log::setTitle("begin_connecting_server-connect");
            if (ec) {
                Log::Log<Log::warning>(ec.message());
                Log::Log<Log::debug>("wait 8.4s to retry connecting server");
                /* when encounter failure in connectiong_server,
                 * most often the server is refusing or the network is down
                 * and thus to sleep for 500ms otherwise it will be too frequent
                 */
                usleep(0x800000);
                begin_connecting_server();
            } else {
                process_client_req(); // send client_connect request
            }
        }
    );
}

void Client::process_client_req() {
    Log::setTitle("Client::process_client_req");
    Log::Log<Log::info>("process client req");
    Protocol::Request req;
    req.set_reqtype(Protocol::client_connect);
    send_buf = Protocol::Request::from_req(req);
    mainsock->async_write_some(
        buffer(send_buf),
        [this](const system::error_code &ec, size_t write_bytes_) {
            Log::setTitle("process_client_req-write");
            if (ec) {
                Log::Log<Log::warning>(ec.message());
                Log::Log<Log::debug>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                if (write_bytes_!=send_buf.size()) {
                    Log::Log<Log::warning>("send_buf.size!=write_bytes, maybe error");
                }
                Log::Log<Log::info>("begin process_server_req");
                process_server_req();
            }
        }
    );
}

void Client::process_server_req() {
    Log::setTitle("Client::process_server_req");
    mainsock->async_read_some(
        buffer(receive_buf),
        [this](const system::error_code &ec, size_t read_bytes_) {
            Log::setTitle("process_server_req-read");
            if (ec) {
                Log::Log<Log::warning>(ec.message());
                Log::Log<Log::debug>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                Protocol::Request req;
                req = Protocol::Request::from_string(receive_buf.substr(0,read_bytes_));
                if (req.get_reqtype()==Protocol::server_connect) {
                    Log::Log<Log::warning>("server_connect, begin connecting target");
                    state = connected;
                    begin_connecting_target();
                } else {
                    Log::Log<Log::warning>(
                        "server request invalid, retry begin_connecting_server"
                    );
                    begin_connecting_server();
                }
            }
        }
    );
}

void Client::begin_connecting_target() {
    Log::setTitle("Client::begin_connecting_target");
    Log::Log<Log::info>("begin connecting server");
    sock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
    // keep_alive(sock);
    sock->async_connect(
        target_ep,
        [this](const system::error_code &ec) {
            Log::setTitle("begin_connecting_target-connect");
            if (ec) {
                Log::Log<Log::warning>(ec.message());
                Log::Log<Log::debug>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                Log::Log<Log::info>("create session");
                auto sess = std::make_shared<Session>(mainsock,sock,session_count++);
                sess->run();
                // give up socket ownership 
                mainsock.reset();
                sock.reset();
                // restart connecting_server for next session
                Log::setTitle("begin_connecting_target-connect");
                Log::Log<Log::info>("new epoch, begin_connecting_server");
                begin_connecting_server();
            }
        }
    );
}


int main(int argc, char *argv[]){
    Log::setLogLevel(Log::info);
    Log::setTitle("main");
    if (argc<5) {
        Log::Log<Log::none>(std::string(argv[0])+
            std::string(" <session ip> <session port> <target ip> <target port>"));
        Log::Log<Log::none>(std::string("session for Server connection, target for target"));
        exit(1);
    }
    std::string sip = argv[1], sport = argv[2], tip = argv[3], tport = argv[4];
    if (sip.find(':')==std::string::npos) {
        sip = std::string("::ffff:").append(sip);
    }
    uint16_t sport_n = std::atoi(sport.c_str());
    if (tip.find(':')==std::string::npos) {
        tip = std::string("::ffff:").append(tip);
    }
    uint16_t tport_n = std::atoi(tport.c_str());
    ignore_sigpipe();
    Client c(sip,sport_n,tip,tport_n);
    c.run();
    return 0;
}
