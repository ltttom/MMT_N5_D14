#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex> // Thêm thư viện mutex
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void handle_client(SOCKET client_socket, vector<pair<string, int>>& peer_list, mutex& peer_mutex) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        cerr << "Failed to receive data or client disconnected unexpectedly." << endl;
        return;
    }

    string peer_info(buffer);
    size_t colon_pos = peer_info.find(":");
    string ip = peer_info.substr(0, colon_pos);
    int port = stoi(peer_info.substr(colon_pos + 1));

    {
        lock_guard<mutex> lock(peer_mutex);
        peer_list.emplace_back(ip, port);
        cout << "New peer registered: " << ip << ":" << port << endl;
    }

    // Gửi danh sách peer về cho client
    string peer_list_str;
    {
        lock_guard<mutex> lock(peer_mutex);
        for (const auto& peer : peer_list) {
            peer_list_str += peer.first + ":" + to_string(peer.second) + ";";
        }
    }
    send(client_socket, peer_list_str.c_str(), peer_list_str.size(), 0);

    // Chờ thêm một thời gian để tránh mất kết nối quá sớm
    this_thread::sleep_for(chrono::seconds(1));

    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    cout << "Server listening on port " << SERVER_PORT << endl;

    vector<pair<string, int>> peer_list;
    mutex peer_mutex; // Khởi tạo mutex để bảo vệ peer_list

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        thread(handle_client, client_socket, ref(peer_list), ref(peer_mutex)).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
