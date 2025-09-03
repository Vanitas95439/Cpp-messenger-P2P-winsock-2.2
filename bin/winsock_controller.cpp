#include <iostream>

#include "winsock_controller.h"
#include "diffie_hellman.h"

Winsock_controller::Winsock_controller() {
    WSAData wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        // error
    }
}

Winsock_controller::~Winsock_controller() {
    if (g_recv_thread_started) {
        g_recv_thread_started = false;
        if (g_recv_thread.joinable()) g_recv_thread.join();
    }
    
    if (g_peer_sock != INVALID_SOCKET) {
        closesocket(g_peer_sock);
    }
    
    WSACleanup();
}
    
int Winsock_controller::controller_connect(std::string ip, int port, bool is_server) {
    // Saving IP and Port
    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (is_server) {
        if(start_listen(addr)) {return 1;}
    } else {
        if(start_connect(addr)) {return 1;}
    }

    // start recv thread
    if (!g_recv_thread_started) {
        g_recv_thread_started = true;
        if (g_recv_thread.joinable()) g_recv_thread.join();
        g_recv_thread = std::thread([this](){ this->peer_recv_loop();});
    }
        
    return 0;
}

void Winsock_controller::send_msg(std::string msg) {
    send(g_peer_sock, msg.c_str(), msg.size() + 1, 0);
} 

SOCKET Winsock_controller::get_socket() {
    return g_peer_sock;
}

// shit
void Winsock_controller::peer_recv_loop() {
    char msg[256];
    while (g_recv_thread_started) {
        int ret = recv(g_peer_sock, msg, sizeof(msg), 0);
        if (ret > 0) {
            std::cout << "\b\b[peer] " << msg << "\n> ";
        }
        
    }
    return;
}

int Winsock_controller::start_listen(SOCKADDR_IN addr) {
    int size_of_addr = sizeof(addr);
    
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    bind(sListen, (SOCKADDR*)&addr, size_of_addr);
    listen(sListen, SOMAXCONN);

    g_peer_sock = accept(sListen, (SOCKADDR*)&addr, &size_of_addr);
    closesocket(sListen);

    if (g_peer_sock != INVALID_SOCKET) {
        // OK
        return 0;
    } else {
        // error
        return 1;
    }
}
    
int Winsock_controller::start_connect(SOCKADDR_IN addr) {
    int size_of_addr = sizeof(addr);

    g_peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (connect(g_peer_sock, (SOCKADDR*)&addr, size_of_addr) == 0) {
        // OK
        return 0;
    } else {
        // error
        return 1;
    }
}