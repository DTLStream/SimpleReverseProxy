#include "Host.h"
#include "Log.h"

const size_t Host::default_bufsize = 1024;

Host::Host():
    ioctx(),
    receive_buf(default_bufsize,0),send_buf(default_bufsize,0),
    state(disconnected),mainsock(),sock() {
    Log::setTitle("Host");
    Log::Log<Log::info>("Host created");
}