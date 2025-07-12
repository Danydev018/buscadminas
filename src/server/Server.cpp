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


Server::Server(const std::string& name, int rows, int cols, int mines)
  : roomName(name),
    R(rows), C(cols), M(mines),
    seed(static_cast<uint32_t>(time(nullptr))),
    board(rows,cols,mines,seed)
{}

// … discoveryListener() igual que antes …

void Server::gameLoop() {
  // abrimos socket TCP
  int srvSock = socket(AF_INET,SOCK_STREAM,0);
  sockaddr_in srvAddr{};
  srvAddr.sin_family      = AF_INET;
  srvAddr.sin_port        = htons(GAME_PORT_BASE);
  srvAddr.sin_addr.s_addr = INADDR_ANY;
  bind(srvSock,(sockaddr*)&srvAddr,sizeof(srvAddr));
  listen(srvSock,1);

  // imprimimos tablero host inicial
  std::cout<<"Tablero inicial (host):\n";
  board.print();

  std::cout<<"Esperando cliente…\n";
  int clientSock = accept(srvSock,nullptr,nullptr);
  std::cout<<"Cliente conectado\n";

  // enviamos parámetros completos
  GameInit gi;
  gi.seed = htonl(seed);
  gi.rows = static_cast<uint8_t>(R);
  gi.cols = static_cast<uint8_t>(C);
  gi.mines= static_cast<uint8_t>(M);
  send(clientSock,&gi,sizeof(gi),0);

  bool turnHost = true;
  while (true) {
    Move mv{};
    if (turnHost) {
      // leer comando sólo en tu turno
      Move mv{};
      while (true) {
          std::cout << "Formato: R fila col  |  F fila col\n> ";
          std::string line;
          std::getline(std::cin, line);

          if (line.empty()) {
              std::cout << "Entrada vacía. Intenta de nuevo.\n";
              continue;
          }

          std::istringstream iss(line);
          char cmd;
          int r = -1, c = -1;
          if (!(iss >> cmd >> r >> c)) {
              std::cout << "Formato inválido. Debes escribir: R 2 3\n";
              continue;
          }

          if (r < 0 || r >= board.rows() || c < 0 || c >= board.cols()) {
              std::cout << "Coordenadas fuera del tablero (" << r << "," << c << ")\n";
              continue;
          }

          cmd = std::toupper(cmd);
          if (cmd == 'R') {
              mv.row    = r;
              mv.col    = c;
              mv.isFlag = 0;
              break;
          } else if (cmd == 'F') {
              mv.row    = r;
              mv.col    = c;
              mv.isFlag = 1;
              break;
          } else {
              std::cout << "Comando inválido. Usa R o F.\n";
          }
      }

      send(clientSock,&mv,sizeof(mv),0);
    } else {
      recv(clientSock,&mv,sizeof(mv),0);
    }

    // aplicamos reveal o flag
    if (mv.isFlag)    board.toggleFlag(mv.row,mv.col);
    else              board.reveal(mv.row,mv.col);

    clearScreen(); board.print();

    // chequeo fin
    if (!mv.isFlag && board.isMine(mv.row,mv.col)) {
      std::cout<<(turnHost?"Has perdido\n":"Has ganado\n");
      break;
    }
    if (board.allSafeRevealed()) {
      std::cout<<(turnHost?"Has ganado\n":"Has perdido\n");
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
