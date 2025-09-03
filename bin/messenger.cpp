#include <iostream>
#include <string>
#include <sstream>
#include <mutex>

#include "winsock_controller.h"

std::mutex g_cout_mtx;

int main() {
    {
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "Simple Messenger\n"
                  << "Usage:\n"
                  << "  cmd listen <ip> <port>  - start listening\n"
                  << "  cmd connect <ip> <port> - connect to server\n"
                  << "  cmd exit                - exit program\n"
                  << "  <message>               - send message\n"
                  << "  you can use only first letter of command\n";
    }

    bool running = true;
    Winsock_controller inet (running, g_cout_mtx);

    std::string line;
    while (running) {
        {
            std::lock_guard<std::mutex> lk(g_cout_mtx);
            std::cout << "> ";
        }

        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss (line);
        std::string cmd; iss >> cmd;
        if (cmd == "cmd") {
            iss >> cmd;
            if (cmd == "listen" || cmd == "l") {
                if (inet.get_socket() != INVALID_SOCKET) {
                    std::lock_guard<std::mutex> lk(g_cout_mtx);
                    std::cout << "You have another connection\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;

                if (ip == "1") {ip = "127.0.0.1"; port = 8080;}

                if (inet.controller_connect(ip, port, 1) == 1) {
                    std::lock_guard<std::mutex> lk(g_cout_mtx);
                    std::cout << "[state] listen error\n";
                }
            } else if (cmd == "connect" || cmd == "c") {
                if (inet.get_socket() != INVALID_SOCKET) {
                    std::cout << "You have another connection\n";
                    break;
                }

                std::string ip; int port; iss >> ip >> port;

                if (ip == "1") {ip = "127.0.0.1"; port = 8080;}

                if (inet.controller_connect(ip, port, 0) == 1) {
                    std::lock_guard<std::mutex> lk(g_cout_mtx);
                    std::cout << "[state] connect error\n";
                }
            } else if (cmd == "exit" || cmd == "e") {
                running = false;
            } else {
                std::lock_guard<std::mutex> lk(g_cout_mtx);
                std::cout << "[state] unknown command: " << cmd << "\n";
            }
        } else {
            if (inet.get_socket() != INVALID_SOCKET) {
                inet.send_msg(line);
            } else {
                std::lock_guard<std::mutex> lk(g_cout_mtx);
                std::cout << "[state] no g_peer_sock\n";
            }
        }
    }

    return 0;
}