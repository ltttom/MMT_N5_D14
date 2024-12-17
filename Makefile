CXX = g++
CXXFLAGS = -pthread -std=c++11

all:server peers

server: server_main.o server.o
	$(CXX) $(CXXFLAGS) server_main.o server.o -o server

server_main.o: server_main.cpp
	$(CXX) $(CXXFLAGS) -c server_main.cpp

server.o: server.cpp
	$(CXX) $(CXXFLAGS) -c server.cpp

peers: peers_main.o peers.o
	$(CXX) $(CXXFLAGS) peers_main.o peers.o -o peers 

peers_main.o: peers_main.cpp
	$(CXX) $(CXXFLAGS) -c peers_main.cpp

peers.o: peers.cpp
	$(CXX) $(CXXFLAGS) -c peers.cpp

clean:
	rm -f core *.o
	rm server client
