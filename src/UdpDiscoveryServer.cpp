#include "UdpDiscoveryServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

UdpDiscoveryServer::UdpDiscoveryServer(const std::string& hostName, int tcpPort, int udpPort)
    : hostName(hostName), tcpPort(tcpPort), udpPort(udpPort), running(false) {}

void UdpDiscoveryServer::start() {
    running = true;
    serverThread = std::thread([this]() { run(); });
}

void UdpDiscoveryServer::stop() {
    running = false;
    if (serverThread.joinable()) serverThread.join();
}

void UdpDiscoveryServer::run() {
    using boost::asio::ip::udp;
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), udpPort));
    char data[1024];
    while (running) {
        udp::endpoint remote_endpoint;
        boost::system::error_code ec;
        size_t len = socket.receive_from(boost::asio::buffer(data), remote_endpoint, 0, ec);
        if (!ec && len > 0) {
            std::string msg(data, len);
            if (msg == "BUSCAMINAS_DISCOVERY") {
                std::string response = hostName + ":" + std::to_string(tcpPort);
                socket.send_to(boost::asio::buffer(response), remote_endpoint);
            }
        }
    }
}
