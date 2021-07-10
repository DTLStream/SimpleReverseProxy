#include "ProxyProto.h"

namespace Protocol {
    // create a Request from string
    Request Request::from_string(const std::string &s) {
        Request req;
        size_t index = 0;
        if (s.size()<sizeof(size_t)) {
            req.reqtype = invalid;
            req.reqlength = SIZE_MAX;
            return req;
        }
        // get length of Request header
        req.reqlength = string2size_t(s);
        index += sizeof(size_t);
        // check for request length
        if (s.size()<req.reqlength) {
            req.reqtype = invalid;
            return req;
        }
        req.reqtype = string2reqtype(s.substr(index));
        index += sizeof(RequestType);
        req.reqcontent = s.substr(index, req.reqlength - index);
        index = req.reqlength;
        if (index<s.size()) {
            req.payload = s.substr(index);
            index += req.payload.size();
        }
        return req;
    }

    // serialize a Request and return a string
    std::string Request::from_req(const Request &req) {
        std::string s;
        std::string temp;
        temp = size_t2string(req.reqlength);
        s.append(temp);
        temp = reqtype2string(req.reqtype);
        s.append(temp);
        s.append(req.reqcontent);
        s.append(req.payload);
        return s;
    }

    // should handle overflow in caller
    size_t Request::string2size_t(const std::string &s) {
        return *reinterpret_cast<const size_t*>
            (s.substr(0,sizeof(size_t)).c_str());
    }

    // should handle overflow in caller
    RequestType Request::string2reqtype(const std::string &s) {
        return *reinterpret_cast<const RequestType*>
            (s.substr(0,sizeof(RequestType)).c_str());
    }

    std::string Request::size_t2string(const size_t size) {
        std::string s;
        const char *val = reinterpret_cast<const char*>(&size);
        s.append(val,val+sizeof(size_t));
        return s;
    }

    std::string Request::reqtype2string(const RequestType &reqt) {
        std::string s;
        const char *val = reinterpret_cast<const char*>(&reqt);
        s.append(val,val+sizeof(RequestType));
        return s;
    }

    Request::Request():reqlength(0),reqtype(invalid),reqcontent(),payload(){
        update_reqlength();
    };

    size_t Request::get_reqlength() const {
        return reqlength;
    }
    
    bool Request::is_invalid() const {
        return reqtype == invalid;
    }
    
    RequestType Request::get_reqtype() const {
        return reqtype;
    }
    
    std::string Request::get_reqstring() const {
        return reqcontent;
    }
    
    std::string Request::get_payload() const {
        return payload;
    }

    void Request::set_reqtype(const RequestType &r) {
        reqtype = r;
        update_reqlength();
    }

    void Request::set_reqcontent(const std::string &c) {
        reqcontent = c;
        update_reqlength();
    }

    void Request::set_payload(const std::string &p){
        payload = p;
        update_reqlength();
    }
    
    void Request::update_reqlength() {
        size_t rl = 0;
        rl += sizeof(reqlength);
        rl += sizeof(reqtype);
        rl += reqcontent.size();
        // rl += payload.size(); // no payload
        reqlength = rl;
    }

}