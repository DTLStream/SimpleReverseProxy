# Simple Reverse Proxy

##  A reverse proxy with multiple connections support, based on Client-Server model,  powered by boost::asio.



### Usage for Server:
Currently not supporting parameters(options), modify IP/port Client/Server's main() to use it

### Usage for Client:
Currently not supporting parameters(options), modify IP/port Client/Server's main() to use it

### Warning
1. Client will keep connecting to the Server if "connection refused", a timer can be used to limit
 the connection frequency
2. If Client-Server connection is lost, next time the user tries to connect to the Server may
 directly get an EOF, and Server may get a Bad Descriptor warning, such errors are due to
    Server/Client's simplified implementation. Connect again and problem may be solved.

### Todo
- [ ] Update logging level (currently the levels are the same)
- [ ] Client Timer (limit retry rates when Client/Server connection is lost)
- [ ] Server/Client Authentication (Passwords in Protocol::Request)
- [ ] ~~TLS(hehe)~~

