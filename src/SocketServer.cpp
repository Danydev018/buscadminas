
#include "SocketServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int SocketServer::getNumClients() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    return clients.size();
}
#include "SocketServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

SocketServer::SocketServer(int port) : port(port) {}

void SocketServer::start() {
    running = true;
    acceptor = std::make_unique<tcp::acceptor>(io_context, tcp::endpoint(tcp::v4(), port));
    serverThread = std::thread([this]() { acceptClients(); });
}

void SocketServer::stop() {
    running = false;
    io_context.stop();
    if (serverThread.joinable()) serverThread.join();
}

void SocketServer::acceptClients() {
    while (running) {
        auto socket = std::make_shared<tcp::socket>(io_context);
        boost::system::error_code ec;
        acceptor->accept(*socket, ec);
        if (!ec) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(socket);
            std::cout << "Cliente conectado: " << socket->remote_endpoint().address().to_string() << std::endl;
        }
    }
}

void SocketServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& client : clients) {
        boost::system::error_code ec;
        boost::asio::write(*client, boost::asio::buffer(message + "\n"), ec);
    }
}


#include <sstream>

std::vector<std::string> SocketServer::receiveAll() {
    std::vector<std::string> mensajes;
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& client : clients) {
        boost::system::error_code ec;
        while (client->available()) {
            boost::asio::streambuf buf;
            boost::asio::read_until(*client, buf, "\n", ec);
            if (ec) break;
            std::istream is(&buf);
            std::string line;
            std::getline(is, line);
            if (!line.empty()) mensajes.push_back(line);
        }
    }
    return mensajes;
}
