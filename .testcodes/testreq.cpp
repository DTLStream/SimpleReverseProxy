#include "../src/ProxyProto.h"
#include <iostream>
#include <cstring>
int main(){
    using namespace Protocol;
    Request req,req0;
    std::string temp;
    temp = "hello world";
    req.set_payload("hello world");
    req.set_reqtype(check);
    temp = "requesting";
    req.set_reqcontent("AAaaAAaaAAaaAAaaAAaaAAaaAAaa");
    req.set_reqcontent("requesting");
    std::string s = Request::from_req(req);
    std::cout<<s.size()<<req.get_reqlength()<<"\n";

    Request req2 = Request::from_string(s);
    bool bitwisesame = !std::memcmp(
        reinterpret_cast<const char*>(&req),reinterpret_cast<const char*>(&req2),sizeof(Request)
    );
    std::cout<<bitwisesame<<"\n";
    return 0;
}
