#pragma once
#include <string>
#include <thread>
#include <atomic>

class UdpDiscoveryServer {
public:
    UdpDiscoveryServer(const std::string& hostName, int tcpPort, int udpPort = 50001);
    void start();
    void stop();
private:
    std::string hostName;
    int tcpPort;
    int udpPort;
    std::atomic<bool> running;
    std::thread serverThread;
    void run();
};
