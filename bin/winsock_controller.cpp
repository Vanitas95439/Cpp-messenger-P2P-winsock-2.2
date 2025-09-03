#include <iostream>

#include "winsock_controller.h"
#include "diffie_hellman.h"

Winsock_controller::Winsock_controller(bool& running_ref, std::mutex& cout_mtx) 
: running{running_ref}, g_cout_mtx{cout_mtx} {
    WSAData wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[WSA] WSAStartup failed.\n";
    }
}

Winsock_controller::~Winsock_controller() {
    if (g_recv_thread.joinable()) g_recv_thread.join();
    
    if (g_peer_sock != INVALID_SOCKET) {
        closesocket(g_peer_sock);
    }
    
    WSACleanup();
}
    
int Winsock_controller::controller_connect(std::string ip, int port, bool is_server) {
    if (port <= 0 || port > 65535) { 
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] invalid port\n";
        return 1; 
    }
    if (ip.empty()) { 
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] invalid ip\n";
        return 1; 
    }

    // Saving IP and Port
    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    // connect or listen
    if (is_server) {
        if(start_listen(addr)) {return 1;}
    } else {
        if(start_connect(addr)) {return 1;}
    }

    std::lock_guard<std::mutex> lk(g_cout_mtx);
    std::cout << "[state] connected\n";

    // start recv thread
    if (g_recv_thread.joinable()) g_recv_thread.join();
    g_recv_thread = std::thread([this](){ this->peer_recv_loop();});
        
    return 0;
}

void Winsock_controller::send_msg(std::string msg) {
    send(g_peer_sock, msg.c_str(), msg.size() + 1, 0);
} 

SOCKET Winsock_controller::get_socket() {
    return g_peer_sock;
}

void Winsock_controller::peer_recv_loop() {
    char msg[256];
    while (running) {
        int ret = recv(g_peer_sock, msg, sizeof(msg), 0);
        if (ret > 0) {
            std::lock_guard<std::mutex> lk(g_cout_mtx);
            std::cout << "\n[peer] " << msg << "\n> ";
        }
        
    }
    return;
}

int Winsock_controller::start_listen(SOCKADDR_IN addr) {
    int size_of_addr = sizeof(addr);
    
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    
    if (bind(sListen, (SOCKADDR*)&addr, size_of_addr) == SOCKET_ERROR) {
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] bind error\n";
        closesocket(sListen);
        return 1;
    }
    
    if (listen(sListen, SOMAXCONN) == SOCKET_ERROR) {
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] listen error\n";
        closesocket(sListen);
        return 1;
    }

    std::lock_guard<std::mutex> lk(g_cout_mtx);
    std::cout << "[state] waiting for connection...\n";

    g_peer_sock = accept(sListen, (SOCKADDR*)&addr, &size_of_addr);
    closesocket(sListen);

    if (g_peer_sock != INVALID_SOCKET) {
        // OK
        return 0;
    } else {
        // error
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] accept error\n";
        return 1;
    }
}
    
int Winsock_controller::start_connect(SOCKADDR_IN addr) {
    int size_of_addr = sizeof(addr);

    g_peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (g_peer_sock == INVALID_SOCKET) {
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] socket error\n";
        return 1;
    }

    std::lock_guard<std::mutex> lk(g_cout_mtx);
    std::cout << "[state] connecting...\n";

    if (connect(g_peer_sock, (SOCKADDR*)&addr, size_of_addr) == 0) {
        // OK
        return 0;
    } else {
        // error
        std::lock_guard<std::mutex> lk(g_cout_mtx);
        std::cout << "[state] connect error\n";
        return 1;
    }
}