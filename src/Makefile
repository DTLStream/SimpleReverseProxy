CXX=g++

RM=rm

CPPFLAGS=-std=c++11 -pedantic -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wno-unused -I/usr/local/include #  -Werror
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
CPPFLAGS+=-pthread
endif

CommonDeps=ProxyProto.cpp Log.cpp Session.cpp Host.cpp

ServerDeps=Server.cpp

ClientDeps=Client.cpp

all: Server Client

Server: $(CommonDeps) $(ServerDeps)
	$(CXX) $(CPPFLAGS) -oServer $(CommonDeps) $(ServerDeps)

Client: $(CommonDeps) $(ClientDeps)
	$(CXX) $(CPPFLAGS) -oClient $(CommonDeps) $(ClientDeps)

clean:
	$(RM) Server Client
