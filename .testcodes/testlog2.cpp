#include "../src/Log.h"

void anotherlog() {
    Log::Log<Log::warning>("another log");
    Log::Log<Log::info>("another log 2");
}
