#include "../src/Log.h"
extern void anotherlog();
int main(){
    anotherlog();
    Log::setLogLevel(Log::debug);
    Log::setTitle("title 0");
    Log::Log<Log::debug>("hello world");
    anotherlog();
    return 0;
}
