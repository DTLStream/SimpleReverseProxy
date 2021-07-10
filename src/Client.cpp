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

// deprecated methods

// void Client::refresh_server_socket() {
//     Log::setTitle("Client::refresh_server_socket");
//     Log::Log<Log::info>("renew mainsock without connection");
//     mainsock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
// }

// void Client::refresh_target_socket() {
//     Log::setTitle("Client::refresh_server_socket");
//     Log::Log<Log::info>("renew sock without connection");
//     sock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
// }

void Client::run() {
    Log::setTitle("Client::run");
    Log::Log<Log::info>("run client");
    begin_connecting_server();
    Log::Log<Log::info>("ioctx.run");
    ioctx.run();
    Log::Log<Log::info>("Client stopped");
}

void Client::destroy() {
    Log::setTitle("Client::destroy");
    if (destroyed) {
        Log::Log<Log::info>("already destroyed");
    } else {
        Log::Log<Log::info>("Client destroy");
        system::error_code error;
        Log::Log<Log::info>("stop mainsock");
        if (mainsock&&mainsock->is_open()) {
            mainsock->cancel(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
            mainsock->close(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
            mainsock.reset();
        }
        Log::Log<Log::info>("stop sock");
        if (sock&&sock->is_open()) {
            sock->cancel(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
            sock->close(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
        }
        state = destroyed;
    }
}

void Client::begin_connecting_server() {
    Log::setTitle("Client::begin_connecting_server");
    Log::Log<Log::info>("begin connecting server");
    mainsock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
    mainsock->async_connect(
        server_ep,
        [&](const system::error_code &ec) {
            Log::setTitle("begin_connecting_server-connect");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s to retry connecting server");
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
        [&](const system::error_code &ec, size_t write_bytes_) {
            Log::setTitle("process_client_req-write");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                if (write_bytes_!=send_buf.size()) {
                    Log::Log<Log::info>("send_buf.size!=write_bytes, maybe error");
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
        [&](const system::error_code &ec, size_t read_bytes_) {
            Log::setTitle("process_server_req-read");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                Protocol::Request req;
                req = Protocol::Request::from_string(receive_buf.substr(0,read_bytes_));
                if (req.get_reqtype()==Protocol::server_connect) {
                    Log::Log<Log::info>("server_connect, begin connecting target");
                    state = connected;
                    begin_connecting_target();
                } else {
                    Log::Log<Log::info>("server request invalid, retry begin_connecting_server");
                    begin_connecting_server();
                }
            }
        }
    );
}

void Client::begin_connecting_target() {
    Log::setTitle("Client::begin_connecting_target");
    sock = std::make_shared<ip::tcp::socket>(ioctx,ip::tcp::v6());
    sock->async_connect(
        target_ep,
        [&](const system::error_code &ec) {
            Log::setTitle("begin_connecting_target-connect");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry begin_connecting_server");
                begin_connecting_server();
            } else {
                Log::Log<Log::info>("create session");
                auto sess = std::make_shared<Session>(mainsock,sock);
                sess->run();
                // give up socket ownership 
                mainsock.reset();
                sock.reset();
                // restart connecting_server for next session
                Log::Log<Log::info>("new epoch, begin_connecting_server");
                begin_connecting_server();
            }
        }
    );
}



int main(){
    Log::setLogLevel(Log::debug);
    Log::setTitle("main");
    Log::Log<Log::info>("in main");
    Client c("::ffff:127.0.0.1",12345,"::ffff:127.0.0.1",12347);
    c.run();
}