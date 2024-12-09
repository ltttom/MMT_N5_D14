#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void start_server(int port) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        char file_name[BUFFER_SIZE] = {0};
        recv(client_socket, file_name, sizeof(file_name), 0);

        cout << "Receiving file: " << file_name << endl;
        ofstream output_file(file_name, ios::binary);
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received;

        while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
            output_file.write(buffer, bytes_received);
        }

        output_file.close();
        cout << "File '" << file_name << "' received successfully." << endl;
        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
}

void connect_to_server(const string& peer_info, vector<pair<string, int>>& peer_list) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection to server failed!" << endl;
        return;
    }

    send(sock, peer_info.c_str(), peer_info.size(), 0);

    char buffer[BUFFER_SIZE] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    cout << "Received peer list: " << buffer << endl;

    string peer_data(buffer);
    size_t pos = 0;
    while ((pos = peer_data.find(";")) != string::npos) {
        string peer = peer_data.substr(0, pos);
        size_t colon = peer.find(":");
        string ip = peer.substr(0, colon);
        int port = stoi(peer.substr(colon + 1));
        peer_list.emplace_back(ip, port);
        peer_data.erase(0, pos + 1);
    }

    closesocket(sock);
    WSACleanup();
}

void send_file(const string& peer_address, int peer_port, const string& file_name) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in peer_addr = {0};
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_address.c_str(), &peer_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        cerr << "Failed to connect to peer!" << endl;
        return;
    }

    send(sock, file_name.c_str(), file_name.size(), 0);

    ifstream input_file(file_name, ios::binary);
    char buffer[BUFFER_SIZE] = {0};
    while (input_file.read(buffer, sizeof(buffer))) {
        send(sock, buffer, input_file.gcount(), 0);
    }
    send(sock, buffer, input_file.gcount(), 0);

    input_file.close();
    closesocket(sock);
    WSACleanup();

    cout << "File '" << file_name << "' sent successfully to " << peer_address << ":" << peer_port << endl;
}

int main() {
    int port;
    cout << "Enter port for this peer: ";
    cin >> port;

    vector<pair<string, int>> peer_list;
    string peer_info = "127.0.0.1:" + to_string(port);

    thread server_thread(start_server, port);
    connect_to_server(peer_info, peer_list);

    while (true) {
        cout << "\nAvailable peers:" << endl;
        for (size_t i = 0; i < peer_list.size(); i++) {
            cout << i + 1 << ". " << peer_list[i].first << ":" << peer_list[i].second << endl;
        }

        int choice;
        cout << "Select a peer to send file (0 to exit): ";
        cin >> choice;

        if (choice == 0) break;

        string file_name;
        cout << "Enter file name to send: ";
        cin >> file_name;

        send_file(peer_list[choice - 1].first, peer_list[choice - 1].second, file_name);
    }

    server_thread.join();
    return 0;
}
