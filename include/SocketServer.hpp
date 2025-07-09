#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <string>

class SocketServer {
public:
    SocketServer(int port);
    void start();
    void stop();
    void broadcast(const std::string& message);
    std::vector<std::string> receiveAll();
    int getNumClients() const;
private:
    int port;
    bool running = false;
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> clients;
    std::thread serverThread;
    mutable std::mutex clientsMutex;
    void acceptClients();
};
