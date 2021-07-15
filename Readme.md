# Simple Reverse Proxy

##  A reverse proxy with multiple connections support, based on Client-Server model,  powered by boost::asio.



### Usage for Server:

```shell
Server <ip> <port> <bind ip> <bind port>
```

users should connect to ```<bind ip>:<bind port>```

### Usage for Client:

```shell
Client <ip> <port> <target ip> <target port>
```

```<target ip>:<target port>``` would receive connections


### Warning

1. Client will keep connecting to the Server if "connection refused", a timer can be used to limit
   the connection frequency
2. If Client-Server connection is lost, next time the user tries to connect to the Server may
   directly get an EOF, and Server may get a Bad Descriptor warning, such errors are due to
    Server/Client's simplified implementation. Connect again and problem may be solved.

### Todo

- [x] Update logging level
- [ ] Client Timer (limit retry rates when Client/Server connection is lost)
- [ ] Server/Client Authentication (Passwords in Protocol::Request)
- [ ] ~~TLS(hehe)~~
