#ifndef SERVER_H
#define SERVER_H

#include "common/Board.h"
#include "common/NetworkCommon.h"
#include <thread>
#include <string>

class Server {
public:
    Server(const std::string& name, int rows, int cols, int mines);
    void run();

private:
    std::string roomName;
    int         R, C, M;      // filas, columnas, minas
    uint32_t    seed;
    Board       board;
    bool        turnHost{true};

    void discoveryListener();
    void gameLoop();
};

#endif // SERVER_H
