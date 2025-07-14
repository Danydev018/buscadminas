#include "server/Server.h"
#include "common/ConsoleUtils.h"
#include "common/NetworkCommon.h"
#include <arpa/inet.h>
#include <ctime>
#include <cstring>
#include <iostream>
#include <limits>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <chrono>

Server::Server(const std::string& name, int rows, int cols, int mines)
  : roomName(name),
    R(rows), C(cols), M(mines),
    seed(static_cast<uint32_t>(time(nullptr))),
    board(rows,cols,mines,seed)
{}

void Server::discoveryListener() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    while (true) {
        DiscoveryRequest req;
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        int n = recvfrom(sock, &req, sizeof(req), 0, (sockaddr*)&clientAddr, &len);
        if (n == sizeof(req) && req.magic[0] == 'M' && req.magic[1] == 'S') {
            DiscoveryReply rep;
            rep.tcpPort = htons(GAME_PORT_BASE);
            strncpy(rep.name, roomName.c_str(), sizeof(rep.name) - 1);
            sendto(sock, &rep, sizeof(rep), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
        }
    }
}

void Server::gameLoop() {
    int srvSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in srvAddr{};
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port   = htons(GAME_PORT_BASE);
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    bind(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr));
    listen(srvSock, 1);

    clearScreen();
    std::cout << "Tablero inicial (host):\n";
    board.drawGotoxy(4, 2);  // Ejemplo: inicia desde columna 4, fila 2


    std::cout << "Esperando cliente…\n";
    int clientSock = accept(srvSock, nullptr, nullptr);
    std::cout << "Cliente conectado\n";

    GameInit gi;
    gi.seed = htonl(seed);
    gi.rows = static_cast<uint8_t>(R);
    gi.cols = static_cast<uint8_t>(C);
    gi.mines= static_cast<uint8_t>(M);
    send(clientSock, &gi, sizeof(gi), 0);

    bool turnHost = true;
    while (true) {
        Move mv{};
        if (turnHost) {
            int cursorRow = 0, cursorCol = 0;
            int lastRow = -1, lastCol = -1;

            while (true) {
                KeyCode key = getKey();

                if (key == KEY_UP    && cursorRow > 0) cursorRow--;
                else if (key == KEY_DOWN  && cursorRow < board.rows() - 1) cursorRow++;
                else if (key == KEY_LEFT  && cursorCol > 0) cursorCol--;
                else if (key == KEY_RIGHT && cursorCol < board.cols() - 1) cursorCol++;

                if (cursorRow != lastRow || cursorCol != lastCol) {
                    lastRow = cursorRow;
                    lastCol = cursorCol;

                    clearScreen();
                    gotoxy(1, 1);
                    drawFrameAroundBoard(4, 2, board.cols(), board.rows());
                    board.drawGotoxy(4, 2);

                    gotoxy(4 + cursorCol * 4, 2 + cursorRow);
                    std::cout << "\033[35m[◉]\033[0m";

                    gotoxy(2, board.rows() + 3);
                    std::cout << "⬆️⬇️⬅️➡️ = moverse | R = revelar | F = bandera | Q = salir";
                }

                if (key == KEY_FLAG) {
                    mv.row = static_cast<uint8_t>(cursorRow);
                    mv.col = static_cast<uint8_t>(cursorCol);
                    mv.isFlag = 1;
                    break;
                }
                else if (key == KEY_ENTER) {
                    mv.row = static_cast<uint8_t>(cursorRow);
                    mv.col = static_cast<uint8_t>(cursorCol);
                    mv.isFlag = 0;
                    break;
                }
                else if (key == KEY_QUIT) {
                    gotoxy(2, board.rows() + 5);
                    std::cout << "🔚 Saliendo del juego...";
                    close(clientSock);
                    close(srvSock);
                    return;
                }
            }
            drawStatusBar("⏳ Esperando movimiento del rival…", board.rows() + 5);
            send(clientSock, &mv, sizeof(mv), 0);
            

        } else {
            
            recv(clientSock, &mv, sizeof(mv), 0);

            // pequeño highlight visual
            gotoxy(1,1);
            std::cout << "🔁 Movimiento rival en (" << mv.row << "," << mv.col << ")";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        if (mv.isFlag) {
            board.toggleFlag(mv.row, mv.col);
            updateBoardDisplay(4, 2, board);  // redibuja con estilo retro

        } else {
            board.reveal(mv.row, mv.col);
            updateBoardDisplay(4, 2, board);  // redibuja con estilo retro
        }
        clearScreen();
        gotoxy(1,1);
        drawFrameAroundBoard(4, 2, board.cols(), board.rows());  // o board->cols() si estás en cliente
        board.drawGotoxy(4, 2);

        if (!mv.isFlag && board.isMine(mv.row, mv.col)) {
            std::cout << (turnHost ? "Has perdido💣\n" : "Has ganado🏁\n");
            break;
        }
        if (board.allSafeRevealed()) {
            std::cout << (turnHost ? "Has ganado🏁\n" : "Has perdido💣\n");
            break;
        }
        turnHost = !turnHost;
    }

    close(clientSock);
    close(srvSock);
}

void Server::run() {
    std::thread disco(&Server::discoveryListener, this);
    disco.detach();
    gameLoop();
}
