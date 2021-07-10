#include "Server.h"
#include "Log.h"
#include "Session.h"
using namespace boost;
using namespace boost::asio;

const size_t Server::backlog = 128;

// may throw due to error in initialization
Server::Server(
        const std::string &cip, uint16_t cport,
        const std::string &uip, uint16_t uport
    ):
    Host(),
    client_ep(ip::address::from_string(cip),cport),client_accpt(ioctx),
    user_ep(ip::address::from_string(uip),uport),user_accpt(ioctx) {
    Log::setTitle("Server");
    Log::Log<Log::info>("Server created");
};

void Server::run() {
    Log::setTitle("Server::run");
    Log::Log<Log::info>("run server");
    system::error_code error;

    client_accpt.open(ip::tcp::v6(),error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }
    client_accpt.bind(client_ep,error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }
    client_accpt.listen(backlog, error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }

    user_accpt.open(ip::tcp::v6(),error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }
    user_accpt.bind(user_ep,error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }
    user_accpt.listen(backlog, error);
    if (error) {
        Log::Log<Log::info>(error.message());
        destroy();
        return;
    }

    Log::Log<Log::info>("begin accepting client");
    begin_accepting_client();

    Log::Log<Log::info>("ioctx.run");
    ioctx.run();
    Log::Log<Log::info>("Server stopped");
}

void Server::destroy() {
    Log::setTitle("Server::destroy");
    if (destroyed) {
        Log::Log<Log::info>("already destroyed");
    } else {
        Log::Log<Log::info>("Server destroy");
        system::error_code error;
        Log::Log<Log::info>("stop client_accpt");
        if (client_accpt.is_open()) {
            client_accpt.cancel(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
            client_accpt.close(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
        }
        Log::Log<Log::info>("stop user_accpt");
        if (user_accpt.is_open()) {
            user_accpt.cancel(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
            user_accpt.close(error);
            if (error) {
                Log::Log<Log::info>(error.message());
            }
        }
        // Log::Log<Log::info>("stop sockets if opened");
        state = destroyed;
    }
}

void Server::begin_accepting_client() {
    Log::setTitle("Server::begin_accepting_client");
    mainsock = std::make_shared<ip::tcp::socket>(ioctx);
    Log::Log<Log::info>("begin accepting client");
    // system::error_code error;
    // cannot set option on unopened socket
    // socket_base::keep_alive keepalive_option(true);
    // mainsock->set_option(keepalive_option,error);
    // if (error) {
    //     // Log
    //     sleep(1); // Debug
    // }
    client_accpt.async_accept(
        *mainsock,
        [&](const system::error_code &ec){
            Log::setTitle("begin_accepting_client-accept");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry Server::begin_accepting_client");
                // mainsock.reset();
                begin_accepting_client();
            } else {
                process_client_req();
            }
        }
    );
}

void Server::process_client_req() {
    Log::setTitle("Server::process_client_req");
    mainsock->async_read_some(
        buffer(receive_buf),
        [&](const system::error_code &ec, size_t read_bytes_) {
            Log::setTitle("process_client_req-read");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry");
                begin_accepting_client();
            } else {
                Protocol::Request req =
                    Protocol::Request::from_string(receive_buf.substr(0,read_bytes_));
                if (req.get_reqtype()==Protocol::client_connect) {
                    Log::Log<Log::info>("client_connect");
                    state = connected;
                    begin_accepting_user();
                } else {
                    Log::Log<Log::info>("client request invalid, retry begin_accepting_client");
                    begin_accepting_client();
                }
            }
        }
    );
}

void Server::begin_accepting_user() {
    Log::setTitle("Server::begin_accepting_user");
    // Log::Log<Log::info>("sock = makeshared");
    sock = std::make_shared<ip::tcp::socket>(ioctx);
    Log::Log<Log::info>("begin accepting user");
    // system::error_code error;
    // cannot set option on unopened socket
    // socket_base::keep_alive keepalive_option(true);
    // sock->set_option(keepalive_option,error);
    // if (error) {
    //     // Log
    //     sleep(1); // Debug
    // }
    user_accpt.async_accept(
        *sock,
        [&](const system::error_code &ec){
            Log::setTitle("begin_accepting_user-accept");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry begin_accepting_user");
                begin_accepting_user();
            } else {
                process_server_req();
            }
        }
    );
}

void Server::process_server_req() {
    Log::setTitle("Server::process_server_req");
    Log::Log<Log::info>("begin process_server_req");
    Protocol::Request req;
    req.set_reqtype(Protocol::server_connect);
    send_buf = Protocol::Request::from_req(req);
    mainsock->async_write_some(
        buffer(send_buf),
        [&](const system::error_code &ec, size_t write_bytes_) {
            Log::setTitle("process_server_req-write");
            if (ec) {
                Log::Log<Log::info>(ec.message());
                Log::Log<Log::info>("wait 0s and retry");
                // mainsock.reset();
                // sock.reset();
                // begin_accepting_client();
            } else {
                if (write_bytes_!=send_buf.size()) {
                    Log::Log<Log::info>("send_buf.size!=write_bytes, maybe error");
                }
                Log::Log<Log::info>("create session");
                auto sess = std::make_shared<Session>(mainsock,sock);
                sess->run();
            }
            mainsock.reset();
            sock.reset();
            state = disconnected;
            begin_accepting_client();
        }
    );
}


int main(int argc, char *argv[]){
    Log::setLogLevel(Log::debug);
    Log::setTitle("main");
    if (argc<5) {
        Log::Log<Log::info>(std::string(argv[0])+
            std::string(" <session ip> <session port> <target ip> <target port>"));
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
    Server s(sip,sport_n,tip,tport_n);
    s.run();
    return 0;
}