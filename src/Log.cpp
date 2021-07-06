#include "Log.h"

namespace Log {
    LogObject::LogObject(const std::string &t, const LogLevel &l, std::ostream &o):
        title(t),level(l),out(o){};
    
    void setLogLevel(LogLevel l) {
        Level = l;
    }
    void setTitle(const std::string &title) {
        Lobj.title = title;
    }
    LogLevel Level(warning);
    const char *default_title = "No Title";
    LogObject Lobj;
    const char *timeformat = "[%b-%d-%Y %H:%M:%S] ";
}


