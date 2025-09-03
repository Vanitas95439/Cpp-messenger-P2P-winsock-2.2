#include <iostream>
#include <string>
#include <sstream>
#include <mutex>

#include "winsock_controller.h"

std::mutex g_cout_mtx;
#define lock_cout std::lock_guard<std::mutex> lk(g_cout_mtx)

int main() {
    Winsock_controller inet {};

    bool running = true;

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
            if (cmd == "listen") {
                if (inet.get_socket() != INVALID_SOCKET) {
                    std::cout << "You have another connection\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;
                if (inet.controller_connect(ip, port, 1) == 1) {
                    lock_cout;
                    std::cout << "[state] listen error\n";
                }
            } else if (cmd == "connect") {
                if (inet.get_socket() != INVALID_SOCKET) {
                    std::cout << "You have another connection\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;
                if (inet.controller_connect(ip, port, 0) == 1) {
                    lock_cout;
                    std::cout << "[state] connect error\n";
                }
            } else if (cmd == "exit") {
                running = false;
            } else {
                lock_cout;
                std::cout << "[state] unknown command: " << cmd << "\n";
            }
        } else {
            if (inet.get_socket() != INVALID_SOCKET) {
                inet.send_msg(line);
            } else {
                lock_cout;
                std::cout << "[state] no g_peer_sock\n";
            }
        }
    }

    return 0;
}