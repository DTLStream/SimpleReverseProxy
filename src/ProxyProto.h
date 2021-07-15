#ifndef PROXYPROTO_H
#define PROXYPROTO_H

#include <string>

namespace Protocol {
    enum RequestType {
        reserved = 0,
        client_connect = 1,
        server_connect = 2,
        check = 3, // not used
        invalid = 4 // not send by Server or Client, indicating that the request is invalid
    };
    
    /*
    * a Request is a simple protocol(header) in both Server and Client
    * the Request is not portable,
    * since size_t and RequestType may vary on different platforms
    */
    class Request {
        public:
        // static utilities
        // create Request from string
        static Request from_string(const std::string &s);
        // create string from Request, which can be sent over socket
        static std::string from_req(const Request &req);

        // reinterpret, not "atoi"
        static size_t string2size_t(const std::string &s);
        static RequestType string2reqtype(const std::string &s);
        static std::string  size_t2string(const size_t size);
        static std::string reqtype2string(const RequestType &reqt);

        // member functions
        Request();
        bool is_invalid() const;
        size_t get_reqlength() const;
        RequestType get_reqtype() const;
        std::string get_reqstring() const;
        std::string get_payload() const;
        void set_reqtype(const RequestType &r);
        void set_reqcontent(const std::string &c);
        void set_payload(const std::string &p);
        private:
        // member var
        size_t reqlength;
        RequestType reqtype;
        std::string reqcontent;
        std::string payload;
        // member functions
        void update_reqlength();
    } /*__attribute__((packed))*/; 
    
}

#endif
