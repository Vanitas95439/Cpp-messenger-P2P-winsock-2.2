#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <map>

// Winsock 2
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

static SOCKET g_peer_sock = INVALID_SOCKET;

std::mutex g_cout_mtx;
#define lock_cout std::lock_guard<std::mutex> lk(g_cout_mtx)

std::atomic<bool> running{true};

std::thread g_recv_thread;

int error(std::string type, std::string error) {
    lock_cout;
    std::cerr << '[' << type << ']' << ' ' << error << '\n';
    return 1;
}

int peer_recv_loop() {
    char msg[256];
    while (running) {
        int ret = recv(g_peer_sock, msg, sizeof(msg), 0);
        if (ret > 0) {
            lock_cout;
            std::cout << "\b\b[peer] " << msg << "\n> ";
        } else {
            lock_cout;
            std::cout << "Connection closed\n";
            break;
        }
    }
    return 0;
}

void start_recv_thread() {
    if (g_recv_thread.joinable()) g_recv_thread.join();
    g_recv_thread = std::thread(peer_recv_loop);
}

int start_listen(std::string ip, int port) {
    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    int size_of_addr = sizeof(addr);

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    bind(sListen, (SOCKADDR*)&addr, size_of_addr);
    listen(sListen, SOMAXCONN);

    g_peer_sock = accept(sListen, (SOCKADDR*)&addr, &size_of_addr);
    closesocket(sListen);

    if (g_peer_sock != INVALID_SOCKET) {
        std::cout << "Client connected\n";
    } else {
        return error("state", "listen error");;
    }

    start_recv_thread();
    return 0;
}

int start_connect (std::string ip, int port) {
    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    int size_of_addr = sizeof(addr);

    g_peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (connect(g_peer_sock, (SOCKADDR*)&addr, size_of_addr) == 0) {
        std::cout << "OK\n";
    } else {
        return error("state", "listen error");
    }

    start_recv_thread();
    return 0;
}

int main() {
    WSAData wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return error("WSA", "WSAStartup error");
    }

    std::map <std::string, int> commands;
    commands["listen"] = 0;
    commands["connect"] = 1;
    commands["exit"] = 2;  
    

    std::string line;
    while (running) {
        {
            lock_cout;
            std::cout << "> ";
        }
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss (line);
        std::string cmd; iss >> cmd;
        if (cmd == "cmd") {
            iss >> cmd;
            switch (commands[cmd])
            {
            case 0: // listen 
            {
                if (g_peer_sock != INVALID_SOCKET) {
                    std::cout << "You have another connection, if you want to reconnect write <cmd reconnect>\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;
                if (start_listen(ip, port) == 1) {
                    error("state", "listen error");
                }
                break;
            }
            case 1: // connect
            {
                if (g_peer_sock != INVALID_SOCKET) {
                    std::cout << "You have another connection, if you want to reconnect write <cmd reconnect>\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;
                if (start_connect(ip, port) == 1) {
                    error("state", "connect error");
                }
                break;
            }
            case 2: // exit
            {
                running = false;
                break;
            }
            default:
            {
                error("cmd", "unknown command");
                break;
            } 
            }
        } else {
            if (g_peer_sock != INVALID_SOCKET) {
                send(g_peer_sock, line.c_str(), line.size() + 1, 0);
            } else {
                error("state", "no g_peer_sock");
            }
        }
    }

    if (g_recv_thread.joinable()) g_recv_thread.join();
    if (g_peer_sock != INVALID_SOCKET) closesocket(g_peer_sock);
    WSACleanup();

    return 0;

}