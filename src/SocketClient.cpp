#include "SocketClient.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

SocketClient::SocketClient(const std::string& host, int port) : host(host), port(port) {}

void SocketClient::connectToServer() {
    socket = std::make_unique<tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(*socket, endpoints);
    connected = true;
    std::cout << "Conectado al servidor " << host << ":" << port << std::endl;
}

void SocketClient::send(const std::string& message) {
    if (!connected) return;
    boost::system::error_code ec;
    boost::asio::write(*socket, boost::asio::buffer(message + "\n"), ec);
    if (ec) {
        std::cout << "[ERROR] Error enviando mensaje: " << ec.message() << std::endl;
        disconnect();
    }
}

std::string SocketClient::receive() {
    if (!connected) return "";
    boost::asio::streambuf buf;
    boost::system::error_code ec;
    boost::asio::read_until(*socket, buf, "\n", ec);
    if (ec) {
        std::cout << "[ERROR] Error recibiendo mensaje: " << ec.message() << std::endl;
        disconnect();
        return "";
    }
    std::istream is(&buf);
    std::string line;
    std::getline(is, line);
    return line;
}

void SocketClient::disconnect() {
    if (connected && socket) {
        boost::system::error_code ec;
        socket->shutdown(tcp::socket::shutdown_both, ec);
        socket->close(ec);
        connected = false;
    }
}
