#pragma once
#include <vector>
#include <string>

struct ServerInfo {
    std::string hostName;
    std::string ip;
    int port;
};

class UdpDiscoveryClient {
public:
    UdpDiscoveryClient(int udpPort = 50001);
    std::vector<ServerInfo> discover(int timeoutMs = 1000);
};
