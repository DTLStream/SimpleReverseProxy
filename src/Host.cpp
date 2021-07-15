#include "Host.h"
#include "Log.h"

// only used for connecting, real buffersize would be specified in Session
const size_t Host::default_bufsize = 1024;

Host::Host():
    ioctx(),
    receive_buf(default_bufsize,0),send_buf(default_bufsize,0),
    state(disconnected),mainsock(),sock(), session_count(0) {
    Log::setTitle("Host");
    Log::Log<Log::info>("Host created");
}
