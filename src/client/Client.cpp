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
    board->print();
}

void Client::play() {
    bool turnHost = true;
    while (true) {
        Move mv{};
        if (!turnHost) {
            while (true) {
                std::cout << "Formato: R fila col  |  F fila col\n> ";
                std::string line;
                std::getline(std::cin, line);

                std::istringstream iss(line);
                char cmd;
                int r = -1, c = -1;
                if (!(iss >> cmd >> r >> c)) {
                    std::cout << "Entrada inválida. Usa R 2 3\n";
                    continue;
                }

                cmd = std::toupper(cmd);
                if (r < 0 || r >= board->rows() || c < 0 || c >= board->cols()) {
                    std::cout << "Coordenadas fuera del tablero (" << r << "," << c << ")\n";
                    continue;
                }

                mv.row = r;
                mv.col = c;

                if (cmd == 'F') {
                    mv.isFlag = 1;
                } else if (cmd == 'R') {
                    mv.isFlag = 0;
                } else {
                    std::cout << "Comando inválido. Usa R o F.\n";
                    continue;
                }

                break;
            }

            send(sockfd, &mv, sizeof(mv), 0);
        } else {
            recv(sockfd, &mv, sizeof(mv), 0);
            gotoxy(1,1);
            std::cout << "🔁 Movimiento rival en (" << mv.row << "," << mv.col << ")";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // 🛠 Aplicar movimiento localmente
        if (mv.isFlag)
            board->toggleFlag(mv.row, mv.col);
        else
            board->reveal(mv.row, mv.col);

        // 🔃 Actualizar pantalla
        clearScreen();
        gotoxy(1,1);
        board->print();

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
