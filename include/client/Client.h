#ifndef CLIENT_H
#define CLIENT_H

#include "common/Board.h"
#include "common/NetworkCommon.h"
#include <vector>
#include <string>

struct ServerInfo {
    std::string ip;
    uint16_t    port;
    std::string name;
};

class Client {
public:
    Client();
    void discover();
    void connectTo(int index);
    void play();

private:
    Board*                     board{nullptr};
    uint32_t                   seed;
    bool                       turnHost{true};
    int                        R, C, M; // filas, columnas, minas recibidas
    int                        sockfd{-1};
    std::vector<ServerInfo>    servers;

    void listServers() const;
};

#endif // CLIENT_H
