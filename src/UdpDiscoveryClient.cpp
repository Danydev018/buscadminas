#include "UdpDiscoveryClient.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <set>

UdpDiscoveryClient::UdpDiscoveryClient(int udpPort) {}

std::vector<ServerInfo> UdpDiscoveryClient::discover(int timeoutMs) {
    using boost::asio::ip::udp;
    boost::asio::io_context io_context;
    udp::socket socket(io_context);
    socket.open(udp::v4());
    socket.set_option(boost::asio::socket_base::broadcast(true));
    udp::endpoint broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), 50001);
    std::string msg = "BUSCAMINAS_DISCOVERY";
    socket.send_to(boost::asio::buffer(msg), broadcast_endpoint);

    std::vector<ServerInfo> servers;
    std::set<std::string> seen;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < timeoutMs) {
        char data[1024];
        udp::endpoint sender_endpoint;
        boost::system::error_code ec;
        socket.non_blocking(true);
        size_t len = socket.receive_from(boost::asio::buffer(data), sender_endpoint, 0, ec);
        if (!ec && len > 0) {
            std::string response(data, len);
            size_t sep = response.find(":");
            if (sep != std::string::npos) {
                ServerInfo info;
                info.hostName = response.substr(0, sep);
                info.port = std::stoi(response.substr(sep + 1));
                info.ip = sender_endpoint.address().to_string();
                if (seen.insert(info.ip + ":" + std::to_string(info.port)).second)
                    servers.push_back(info);
            }
        }
    }
    return servers;
}
