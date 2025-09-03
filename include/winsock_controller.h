#pragma once
#include <string>
#include <thread>
#include <mutex>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class Winsock_controller {
public:
    Winsock_controller(bool& running_ref, std::mutex& cout_mtx);
    ~Winsock_controller();

    int controller_connect(std::string ip, int port, bool is_server); 
    
    void send_msg(std::string msg);
   
    SOCKET get_socket(); 
private:
    SOCKET g_peer_sock = INVALID_SOCKET;
    
    std::thread g_recv_thread;

    bool& running;
    std::mutex& g_cout_mtx;

    void peer_recv_loop();

    int start_listen(SOCKADDR_IN addr);
    
    int start_connect(SOCKADDR_IN addr);
};



