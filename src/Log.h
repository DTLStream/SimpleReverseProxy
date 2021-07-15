#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
namespace Log {

    // Log is not thread-safe
    // but currently we are merely implementing a single-threaded proxy

    enum LogLevel {
        none = 0,
        error = 1,
        warning = 2,
        info = 3,
        debug = 4
    };

    extern LogLevel Level;

    extern const char *default_title;

    struct LogObject {
        std::string title;
        const LogLevel &level;
        std::ostream &out;
        // LogObject() = delete; // ambiguous if added
        LogObject(const std::string &t = default_title, const LogLevel &l = Level,
            std::ostream &o = std::cerr);
    };

    extern LogObject Lobj;

    void setLogLevel(LogLevel l);

    void setTitle(const std::string &title = default_title);
    
    extern const char * timeformat;

    template <LogLevel L>
    void Log(const std::string &content) {
        // std::chrono::time_point<std::chrono::system_clock, std::milli> timepoint =\
        //     std::chrono::system_clock::now();
        if (L>Lobj.level) return;
        auto timepoint = std::chrono::system_clock::now();
        auto ctimenow = std::chrono::system_clock::to_time_t(timepoint);
        std::string outstring = Lobj.title + " - " + content;
        Lobj.out << std::put_time(std::localtime(&ctimenow),timeformat) << outstring << "\n";
    }
};

#endif
