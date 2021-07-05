CXX=g++
RM=rm
CPPFLAGS=-std=c++11 -pedantic -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Werror -Wno-unused -I/usr/local/include
Server:
	$(CXX) $(CPPFLAGS) -oServer Server.cpp ProxyProto.cpp Log.cpp Session.cpp
Client:
	$(CXX) $(CPPFLAGS) -oClient Client.cpp ProxyProto.cpp Log.cpp Session.cpp
clean:
	$(RM) Server Client

