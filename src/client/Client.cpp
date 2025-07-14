#include "client/Client.h"
#include "common/ConsoleUtils.h"
#include "common/NetworkCommon.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <unistd.h>
#include <sstream>
#include <thread>

Client::Client(): board(nullptr) {}

void Client::discover() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    sockaddr_in bcast{};
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(DISCOVERY_PORT);
    bcast.sin_addr.s_addr = INADDR_BROADCAST;

    DiscoveryRequest req;
    sendto(sock, &req, sizeof(req), 0, (sockaddr*)&bcast, sizeof(bcast));

    auto start = std::chrono::steady_clock::now();
    while (true) {
        DiscoveryReply rep;
        sockaddr_in srvAddr{};
        socklen_t len = sizeof(srvAddr);
        timeval tv{2,0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        if (recvfrom(sock, &rep, sizeof(rep), 0, (sockaddr*)&srvAddr, &len) > 0) {
            ServerInfo si;
            si.ip = inet_ntoa(srvAddr.sin_addr);
            si.port = ntohs(rep.tcpPort);
            si.name = rep.name;
            servers.push_back(si);
        }
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(2))
            break;
    }
    close(sock);
    listServers();
}

void Client::listServers() const {
    if (servers.empty()) {
        std::cout << "No se encontraron servidores en la red.\n";
        return;
    }

    std::cout << "Servidores disponibles:\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << i << ") " << servers[i].name
                  << " @ " << servers[i].ip << ":" << servers[i].port << "\n";
    }
}

void Client::connectTo(int idx) {
    auto& s = servers[idx];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(s.port);
    inet_pton(AF_INET, s.ip.c_str(), &addr.sin_addr);
    connect(sockfd, (sockaddr*)&addr, sizeof(addr));

    GameInit gi;
    recv(sockfd, &gi, sizeof(gi), 0);
    seed = ntohl(gi.seed);
    R = gi.rows;
    C = gi.cols;
    M = gi.mines;

    clearScreen();
    gotoxy(1,1);
    board = new Board(R, C, M, seed);
    board->drawGotoxy(4, 2);  // Ejemplo: inicia desde columna 4, fila 2

}

void Client::play() {
    bool turnHost = true;
    while (true) {
        Move mv{};
        if (!turnHost) {
            int cursorRow = 0, cursorCol = 0;
            int lastRow = -1, lastCol = -1;

            while (true) {
                KeyCode key = getKey();

                if (key == KEY_UP    && cursorRow > 0) cursorRow--;
                else if (key == KEY_DOWN  && cursorRow < board->rows() - 1) cursorRow++;
                else if (key == KEY_LEFT  && cursorCol > 0) cursorCol--;
                else if (key == KEY_RIGHT && cursorCol < board->cols() - 1) cursorCol++;

                if (cursorRow != lastRow || cursorCol != lastCol) {
                    lastRow = cursorRow;
                    lastCol = cursorCol;

                    clearScreen();
                    gotoxy(1, 1);
                    drawFrameAroundBoard(4, 2, board->cols(), board->rows());
                    board->drawGotoxy(4, 2);

                    gotoxy(4 + cursorCol * 4, 2 + cursorRow);
                    std::cout << "\033[35m[◉]\033[0m";

                    gotoxy(2, board->rows() + 3);
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
                    gotoxy(2, board->rows() + 5);
                    std::cout << "🔚 Saliendo del juego...";
                    close(sockfd);
                    delete board;
                    return;
                }
            }

            send(sockfd, &mv, sizeof(mv), 0);
            drawStatusBar("⏳ Esperando movimiento del rival…", board->rows() + 5);

        } else {
            recv(sockfd, &mv, sizeof(mv), 0);
            gotoxy(1,1);
            std::cout << "🔁 Movimiento rival en (" << mv.row << "," << mv.col << ")";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // 🛠 Aplicar movimiento localmente
        if (mv.isFlag){
            board->toggleFlag(mv.row, mv.col);
            updateBoardDisplay(4, 2, *board);

        } else {
            board->reveal(mv.row, mv.col);
            updateBoardDisplay(4, 2, *board);

        }
        // 🔃 Actualizar pantalla
        clearScreen();
        gotoxy(1,1);
        drawFrameAroundBoard(4, 2, board->cols(), board->rows());  // o board->cols() si estás en cliente
        board->drawGotoxy(4, 2);
        

        // 💣 Verificación de bomba
        if (!mv.isFlag && board->isMine(mv.row, mv.col)) {
            std::cout << (!turnHost ? "Has perdido💣\n" : "Has ganado🏁\n");
            break;
        }

        // 🏁 Verificación de victoria
        if (board->allSafeRevealed()) {
            std::cout << (!turnHost ? "Has ganado🏁\n" : "Has perdido💣\n");
            break;
        }

        turnHost = !turnHost;
    }

    close(sockfd);
    delete board;
}
