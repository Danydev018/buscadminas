#include "server/Server.h"
#include "common/ConsoleUtils.h"
#include "common/NetworkCommon.h"
#include <arpa/inet.h>
#include <ctime>
#include <cstring>
#include <iostream>
#include <fstream>
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

Server::~Server() {  
    stop();  
}

bool Server::validateMove(const Move& mv) const {  
    return mv.row < R && mv.col < C && (mv.isFlag == 0 || mv.isFlag == 1);  
}

void Server::stop() {  
    shouldStop.store(true);  
    if (discoveryThread.joinable()) {  
        discoveryThread.join();  
    }  
    if (discoverySocket != -1) {  
        close(discoverySocket);  
    }  
} 

void Server::discoveryListener() {  
    discoverySocket = socket(AF_INET, SOCK_DGRAM, 0);  
    if (discoverySocket == -1) {  
        std::cerr << "Error creando socket UDP" << std::endl;  
        return;  
    }  
      
    // Configurar timeout para recvfrom  
    struct timeval timeout;  
    timeout.tv_sec = 1;  
    timeout.tv_usec = 0;  
    setsockopt(discoverySocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  
      
    sockaddr_in addr{};  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(DISCOVERY_PORT);  
    addr.sin_addr.s_addr = INADDR_ANY;  
      
    if (bind(discoverySocket, (sockaddr*)&addr, sizeof(addr)) == -1) {  
        std::cerr << "Error en bind UDP" << std::endl;  
        close(discoverySocket);  
        return;  
    }  
  
    while (!shouldStop.load()) {  
        DiscoveryRequest req;  
        sockaddr_in clientAddr{};  
        socklen_t len = sizeof(clientAddr);  
          
        int n = recvfrom(discoverySocket, &req, sizeof(req), 0, (sockaddr*)&clientAddr, &len);  
          
        if (n == sizeof(req) && req.magic[0] == 'M' && req.magic[1] == 'S') {  
            DiscoveryReply rep;  
            rep.tcpPort = htons(GAME_PORT_BASE);  
            strncpy(rep.name, roomName.c_str(), sizeof(rep.name) - 1);  
            sendto(discoverySocket, &rep, sizeof(rep), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));  
        }  
        // Si recvfrom da timeout (EAGAIN/EWOULDBLOCK), continúa el bucle  
    }  
      
    close(discoverySocket);  
}

void Server::printScores() const {
    gotoxy(4, 3 + board.rows());
    std::cout << "\033[96mScore Host: " << scoreHost << "   Score Cliente: " << scoreClient << "\033[0m\n";
}

void Server::saveWinnerToCSV(const std::string& winnerName, int score) const {
    std::ofstream file("src/common/output/scores.csv", std::ios::app);
    if (file.is_open()) {
        auto t = std::time(nullptr);
        char fecha[32];
        std::strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M", std::localtime(&t));
        file << winnerName << ',' << score << ',' << fecha << '\n';
        file.close();
    }
}

void Server::gameLoop() {
    int srvSock = socket(AF_INET, SOCK_STREAM, 0);
    int clientSock = -1;
    if (srvSock == -1) {
        std::cerr << "Error creando socket TCP" << std::endl;
        return;
    }

    //reutilizar el puerto
    int opt = 1;
    if (setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Error configurando SO_REUSEADDR" << std::endl;
        close(srvSock);
        return;
    }
      
    // Esperar conexión del cliente antes de iniciar el juego
    sockaddr_in srvAddr{};
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(GAME_PORT_BASE);
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr)) == -1) {
        std::cerr << "Error en bind TCP" << std::endl;
        close(srvSock);
        return;
    }
    if (listen(srvSock, 1) == -1) {
        std::cerr << "Error en listen" << std::endl;
        close(srvSock);
        return;
    }
    clearScreen();
    std::cout << "Tablero inicial (host):\n";
    board.drawGotoxy(4, 2);
    std::cout << "Esperando cliente…\n";
    clientSock = accept(srvSock, nullptr, nullptr);
    if (clientSock == -1) {
        std::cerr << "Error aceptando conexión" << std::endl;
        close(srvSock);
        return;
    }
    std::cout << "Cliente conectado\n";
    // Enviar configuración del juego
    GameInit gi;
    gi.seed = htonl(seed);
    gi.rows = static_cast<uint8_t>(R);
    gi.cols = static_cast<uint8_t>(C);
    gi.mines = static_cast<uint8_t>(M);
    if (NetworkUtils::safeSend(clientSock, &gi, sizeof(gi)) <= 0) {
        std::cerr << "Error enviando configuración del juego" << std::endl;
        close(clientSock);
        close(srvSock);
        return;
    }
    bool turnHost = true;
    while (true) {
        Move mv{};
        int* currentScore = turnHost ? &scoreHost : &scoreClient;
        if (turnHost) {
            // Turno del servidor (host)
            int cursorRow = 0, cursorCol = 0;
            int lastRow = -1, lastCol = -1;
            while (true) {
                KeyCode key = getKey();
                if (key == KEY_UP && cursorRow > 0) cursorRow--;
                else if (key == KEY_DOWN && cursorRow < board.rows() - 1) cursorRow++;
                else if (key == KEY_LEFT && cursorCol > 0) cursorCol--;
                else if (key == KEY_RIGHT && cursorCol < board.cols() - 1) cursorCol++;
                if (cursorRow != lastRow || cursorCol != lastCol) {
                    lastRow = cursorRow;
                    lastCol = cursorCol;
                    clearScreen();
                    gotoxy(1, 1);
                    drawFrameAroundBoard(4, 2, board.cols(), board.rows());
                    board.drawGotoxy(4, 2);
                    printScores();
                    gotoxy(4 + cursorCol * 3, 2 + cursorRow);
                    std::cout << "\033[35m◉\033[0m";
                    gotoxy(2, board.rows() + 6);
                    std::cout << "\033[92m┌─ CONTROLES ─────────────────────────────────────┐\033[0m\n";
                    gotoxy(2, board.rows() + 7);
                    std::cout << "\033[92m│ \033[97m⬆️⬇️⬅️➡️ Mover cursor  \033[93mR\033[97m Revelar  \033[93mF\033[97m Bandera \033[93mQ\033[97m Salir \033[92m│\033[0m\n";
                    gotoxy(2, board.rows() + 8);
                    std::cout << "\033[92m└─────────────────────────────────────────────────┘\033[0m";
                }
                if (key == KEY_FLAG) {
                    mv.row = static_cast<uint8_t>(cursorRow);
                    mv.col = static_cast<uint8_t>(cursorCol);
                    mv.isFlag = 1;
                    break;
                } else if (key == KEY_ENTER) {
                    mv.row = static_cast<uint8_t>(cursorRow);
                    mv.col = static_cast<uint8_t>(cursorCol);
                    mv.isFlag = 0;
                    break;
                } else if (key == KEY_QUIT) {
                    gotoxy(2, board.rows() + 5);
                    std::cout << "🔚 Saliendo del juego...";
                    close(clientSock);
                    close(srvSock);
                    return;
                }
            }
            drawStatusBar("⏳ Esperando movimiento del rival…", board.rows() + 5);
            if (NetworkUtils::safeSend(clientSock, &mv, sizeof(mv)) <= 0) {
                std::cerr << "Error enviando movimiento al cliente" << std::endl;
                break;
            }
        } else {
            int result = NetworkUtils::safeRecv(clientSock, &mv, sizeof(mv), 30);
            if (result <= 0) {
                if (result == -2) {
                    std::cout << "Timeout: El cliente no respondió en 30 segundos" << std::endl;
                } else if (result == 0) {
                    std::cout << "Cliente desconectado" << std::endl;
                } else {
                    std::cout << "Error de red al recibir movimiento del cliente" << std::endl;
                }
                break;
            }
            if (!validateMove(mv)) {
                std::cerr << "Movimiento inválido recibido del cliente: ("
                         << (int)mv.row << "," << (int)mv.col << "), flag=" << (int)mv.isFlag << std::endl;
                continue;
            }
            highlightCell(mv.row, mv.col, mv.isFlag ? "[🚩]" : "[!]");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            gotoxy(1, 1);
            std::cout << "🔁 Movimiento rival en (" << (int)mv.row << "," << (int)mv.col << ")";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // Aplicar movimiento al tablero y actualizar score
        if (mv.isFlag) {
            if (board.at(mv.row, mv.col).hasMine && board.at(mv.row, mv.col).state != Flagged) {
                *currentScore += 100;
            }
            board.toggleFlag(mv.row, mv.col);
        } else {
            int ady = board.at(mv.row, mv.col).adjacentMines;
            if (!board.at(mv.row, mv.col).hasMine && board.at(mv.row, mv.col).state == Hidden) {
                if (ady > 0) *currentScore += (ady + 1) * 100;
            }
            board.reveal(mv.row, mv.col);
        }
        clearScreen();
        gotoxy(1, 1);
        drawFrameAroundBoard(4, 2, board.cols(), board.rows());
        board.drawGotoxy(4, 2);
        printScores();
        if (!mv.isFlag) {
            if (board.isMine(mv.row, mv.col)) {
                std::string result = turnHost ? "Has perdido💣" : "Has ganado🏁";
                showAllMines(board, result);
                break;
            }
            if (board.allSafeRevealed()) {
                std::string result = turnHost ? "Has ganado🏁" : "Has perdido💣";
                showAllMines(board, result);
                break;
            }
        }
        turnHost = !turnHost;
    }

    // Mostrar scores finales y guardar ganador
    clearScreen();
    gotoxy(1, 1);
    drawFrameAroundBoard(4, 2, board.cols(), board.rows());
    board.drawGotoxy(4, 2);
    printScores();
    std::cout << "\n\033[93m--- FIN DE PARTIDA ---\033[0m\n";
    std::string winnerName;
    int winnerScore = 0;
    if (scoreHost > scoreClient) {
        std::cout << "\033[92m¡Gana el Host! Ingresa tu nombre: \033[0m";
        std::cin >> winnerName;
        winnerScore = scoreHost;
    } else if (scoreClient > scoreHost) {
        std::cout << "\033[92m¡Gana el Cliente! Ingresa tu nombre: \033[0m";
        std::cin >> winnerName;
        winnerScore = scoreClient;
    } else {
        std::cout << "\033[94mEmpate. Ingresa nombre del jugador a guardar: \033[0m";
        std::cin >> winnerName;
        winnerScore = scoreHost;
    }
    saveWinnerToCSV(winnerName, winnerScore);
    // Limpieza de recursos
    close(clientSock);
    close(srvSock);
}

void Server::run() {  
    discoveryThread = std::thread(&Server::discoveryListener, this);  
    gameLoop();  
    stop();  
}