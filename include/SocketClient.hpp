#pragma once
#include <boost/asio.hpp>
#include <string>
#include <thread>

class SocketClient {
public:
    SocketClient(const std::string& host, int port);
    void connectToServer();
    void send(const std::string& message);
    std::string receive();
    void disconnect();
private:
    std::string host;
    int port;
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
    std::thread clientThread;
    bool connected = false;
};
