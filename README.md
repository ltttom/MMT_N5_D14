g++ server.cpp -o server -lWs2_32 -pthread
g++ peer.cpp -o peer -lWs2_32 -pthread
./server
./peer
 
 
