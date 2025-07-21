#include "client/Client.h"
#include "server/Server.h"
#include "common/ConsoleUtils.h"
#include "common/NetworkUtils.h"
#include "common/NetworkCommon.h"
#include "common/ScoreSystem.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <unistd.h>
#include <sstream>
#include <thread>

Client::Client(): board(nullptr) {}
Client::~Client() {  
    if (board) {  
        delete board;  
        board = nullptr;  
    }  
    if (sockfd != -1) {  
        close(sockfd);  
    }  
} 

bool Client::validateMove(const Move& mv) const {  
    return mv.row < R && mv.col < C && (mv.isFlag == 0 || mv.isFlag == 1);  
}

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
      
    if (connect(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {  
        std::cerr << "Error conectando al servidor" << std::endl;  
        close(sockfd);  
        return;  
    }  
  
    GameInit gi;  
    int result = NetworkUtils::safeRecv(sockfd, &gi, sizeof(gi), 10);  
    if (result <= 0) {  
        std::cerr << "Error recibiendo configuración del juego" << std::endl;  
        close(sockfd);  
        return;  
    }  
      
    seed = ntohl(gi.seed);  
    R = gi.rows;  
    C = gi.cols;  
    M = gi.mines;  
      
    // Validar parámetros  
    if (R == 0 || C == 0 || M == 0 || R > 50 || C > 50 || M >= R*C) {  
        std::cerr << "Parámetros de juego inválidos" << std::endl;  
        close(sockfd);  
        return;  
    }  
  
    clearScreen();  
    board = new Board(R, C, M, seed);  
    gotoxy(1, 1);  
    drawFrameAroundBoard(4, 2, board->cols(), board->rows()); 
    board->drawGotoxy(4, 2);  
   
}

void Client::play() {  
    if (!board) {  
        std::cerr << "Error: Tablero no inicializado" << std::endl;  
        return;  
    }  
  
    bool turnHost = true;  
    // Variables de puntuación  
    auto startTime = std::chrono::steady_clock::now();  
    int clientClicks = 0, hostClicks = 0;  
    int clientFlags = 0, hostFlags = 0;  
    int difficulty = 2; // Se puede sincronizar con el servidor
    auto lastTimeUpdate = std::chrono::steady_clock::now();  
    bool statsDisplayed = false;

    while (true) {  
        auto currentTime = std::chrono::steady_clock::now();  
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();  
        
        // Actualizar tiempo cada segundo  
        if (std::chrono::duration<double>(currentTime - lastTimeUpdate).count() >= 1.0) {  
            if (statsDisplayed) {  
                ScoreCalculator::updateTimeOnly(elapsedTime, board->rows());  
            }  
            lastTimeUpdate = currentTime;  
        } 
        Move mv{};  
          
        if (!turnHost) {  
            // Turno del cliente - capturar input del usuario  
            int cursorRow = 0, cursorCol = 0;  
            int lastRow = -1, lastCol = -1;  

            while (true) {  
                KeyCode key = getKey();    
  
                if (key == KEY_UP && cursorRow > 0) cursorRow--;  
                else if (key == KEY_DOWN && cursorRow < board->rows() - 1) cursorRow++;  
                else if (key == KEY_LEFT && cursorCol > 0) cursorCol--;  
                else if (key == KEY_RIGHT && cursorCol < board->cols() - 1) cursorCol++;  
  
                if (cursorRow != lastRow || cursorCol != lastCol) {  
                    lastRow = cursorRow;  
                    lastCol = cursorCol;  
  
                    clearScreen();  
                    gotoxy(1, 1);  
                    drawFrameAroundBoard(4, 2, board->cols(), board->rows());  
                    board->drawGotoxy(4, 2);  
                    // Mostrar estadísticas en vivo  
                    ScoreCalculator::displayLiveStats(clientClicks, clientFlags, elapsedTime,  
                                                    hostClicks, hostFlags,  
                                                    "CLIENT", "HOST", board->rows());  
                    statsDisplayed = true;
                    if (board->rows() >= 14) gotoxy(4 + cursorCol * 3, 1 + cursorRow);    
                    else gotoxy(4 + cursorCol * 3, 2 + cursorRow); 
                    std::cout << "\033[35m◉\033[0m";
                    // highlightCell(cursorRow, cursorCol, "[◉]");  
  
                    gotoxy(2, board->rows() + 9);  // Era board->rows() + 6  
                    std::cout << "\033[92m┌─ CONTROLES ──────────────────────────────┐\033[0m\n";  
                    gotoxy(2, board->rows() + 10);  // Era board->rows() + 7  
                    std::cout << "\033[92m│ \033[97m⬆️⬇️⬅️➡️ Mover  \033[93mR\033[97m Revelar  \033[93mF\033[97m Bandera \033[93mQ\033[97m Salir \033[92m│\033[0m\n";  
                    gotoxy(2, board->rows() + 11);  // Era board->rows() + 8  
                    std::cout << "\033[92m└──────────────────────────────────────────┘\033[0m\n";
                    
                }  
  
                if (key == KEY_FLAG) {  
                    mv.row = static_cast<uint8_t>(cursorRow);  
                    mv.col = static_cast<uint8_t>(cursorCol);  
                    mv.isFlag = 1;  
                    clientClicks++;  
                    clientFlags++;  
                    break;  
                }  
                else if (key == KEY_ENTER) {  
                    mv.row = static_cast<uint8_t>(cursorRow);  
                    mv.col = static_cast<uint8_t>(cursorCol);  
                    mv.isFlag = 0;  
                    clientClicks++;  
                    break;  
                }
                else if (key == KEY_QUIT) {  
                    gotoxy(2, board->rows() + 5);  
                    std::cout << "🔚 Saliendo del juego...";  
                    close(sockfd);  
                    delete board;  
                    board = nullptr;  
                    return;  
                }  
            }  
  
            // Enviar movimiento al servidor con verificación de errores  
            if (NetworkUtils::safeSend(sockfd, &mv, sizeof(mv)) <= 0) {  
                std::cerr << "Error enviando movimiento al servidor" << std::endl;  
                break;  
            }   
            
            gotoxy(2, board->rows() + 5);
            std::cout << "⏳ Esperando movimiento del rival…";   
  
        } else {  
            // Turno del servidor - recibir movimiento  
            int result = NetworkUtils::safeRecv(sockfd, &mv, sizeof(mv), 30);  
            if (result <= 0) {  
                if (result == -2) {  
                    std::cout << "Timeout: El servidor no respondió en 30 segundos" << std::endl;  
                } else if (result == 0) {  
                    std::cout << "Servidor desconectado" << std::endl;  
                } else {  
                    std::cout << "Error de red al recibir movimiento del servidor" << std::endl;  
                }  
                break;  
            }  
              
            // Validar movimiento recibido del servidor  
            if (mv.row >= R || mv.col >= C || (mv.isFlag != 0 && mv.isFlag != 1)) {  
                std::cerr << "Movimiento inválido recibido del servidor: ("   
                         << (int)mv.row << "," << (int)mv.col << "), flag=" << (int)mv.isFlag << std::endl;  
                continue; // Continuar esperando un movimiento válido  
            }  

            // Rastrear movimientos del servidor  
            if (mv.isFlag) {  
                hostClicks++;  
                hostFlags++;  
            } else {  
                hostClicks++;  
            }
            // highlightCell(mv.row, mv.col, mv.isFlag ? "[🚩]" : "[!]");  
            // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        }  
  
        // Aplicar movimiento localmente al tablero  
        if (mv.isFlag) {  
            board->toggleFlag(mv.row, mv.col);  
            updateBoardDisplay(4, 2, *board);  
        } else {  
            board->reveal(mv.row, mv.col);  
            updateBoardDisplay(4, 2, *board);  
        }  
        
        // Actualizar pantalla completa  
        clearScreen();  
        gotoxy(1, 1);  
        drawFrameAroundBoard(4, 2, board->cols(), board->rows());  
        board->drawGotoxy(4, 2);  
        
        // SOLO verificar condiciones de fin de juego si fue una revelación  
        if (!mv.isFlag) {  
        if (board->isMine(mv.row, mv.col)) {  
            std::string result = !turnHost ? "Has perdido💣" : "Has ganado🏁";
            showAllMines(*board, result);

            // Recibir la estructura de score del servidor
            MultiplayerScore mscore;
            recv(sockfd, &mscore, sizeof(mscore), MSG_WAITALL);

            GameScore hostScore;
            hostScore.totalScore = mscore.hostScore;
            hostScore.difficulty = difficulty;
            hostScore.gameTimeSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
            hostScore.boardRows = R;
            hostScore.boardCols = C;
            hostScore.totalMines = M;
            hostScore.totalClicks = hostClicks;
            hostScore.flagsUsed = hostFlags;
            // Mapear dificultad numérica a string
            switch (difficulty) {
                case 1: hostScore.difficulty = "facil"; break;
                case 2: hostScore.difficulty = "medio"; break;
                case 3: hostScore.difficulty = "dificil"; break;
                default: hostScore.difficulty = "desconocido"; break;
            }

            GameScore clientScore;
            clientScore.totalScore = mscore.clientScore;
            clientScore.difficulty = difficulty;
            clientScore.gameTimeSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
            clientScore.boardRows = R;
            clientScore.boardCols = C;
            clientScore.totalMines = M;
            clientScore.totalClicks = clientClicks;
            clientScore.flagsUsed = clientFlags;
            switch (difficulty) {
                case 1: clientScore.difficulty = "facil"; break;
                case 2: clientScore.difficulty = "medio"; break;
                case 3: clientScore.difficulty = "dificil"; break;
                default: clientScore.difficulty = "desconocido"; break;
            }

            ScoreCalculator::displayMultiplayerResults(hostScore, clientScore, "HOST", "CLIENT");

            // Determinar ganador real según los puntajes recibidos
            std::string winnerName;
            std::string winnerLabel;
            GameScore* winnerScore = nullptr;
            // Si el hostScore es mayor, ganó el host; si el clientScore es mayor, ganó el cliente
            if (hostScore.totalScore > clientScore.totalScore) {
                winnerLabel = "HOST";
                winnerScore = &hostScore;
            } else {
                winnerLabel = "CLIENT";
                winnerScore = &clientScore;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            gotoxy(2, board ? board->rows() + 15 : 15);
            std::cout << "¡Has ganado! Ingrese su nombre (Enter para '" << winnerLabel << "'): ";
            std::cout.flush();
            std::getline(std::cin, winnerName);
            if (winnerName.empty()) winnerName = winnerLabel;
            winnerScore->won = true;
            ScoreCalculator::saveMultiplayerScoreToCSV(*winnerScore, winnerName);
            gotoxy(2, board ? board->rows() + 17 : 17);
            std::cout << "Puntaje guardado." << std::endl;
            break;
        }  
        
        if (board->allSafeRevealed()) {  
            std::string result = !turnHost ? "Has ganado🏁" : "Has perdido💣";
            showAllMines(*board, result);

            // Calcular puntuaciones para victoria completa
            auto endTime = std::chrono::steady_clock::now();
            double gameTime = std::chrono::duration<double>(endTime - startTime).count();

            bool clientWon = !turnHost;
            GameScore clientScore = ScoreCalculator::calculateScore(difficulty, R, C,
                                                                gameTime, clientClicks, clientFlags, clientWon);

            bool hostWon = turnHost;
            GameScore hostScore = ScoreCalculator::calculateScore(difficulty, R, C,
                                                                gameTime, hostClicks, hostFlags, hostWon);

            ScoreCalculator::displayMultiplayerResults(hostScore, clientScore, "HOST", "CLIENT");

            // Determinar ganador real según las variables de victoria
            std::string winnerName;
            std::string winnerLabel;
            GameScore* winnerScore = nullptr;
            if (clientWon) {
                winnerLabel = "CLIENT";
                winnerScore = &clientScore;
            } else {
                winnerLabel = "HOST";
                winnerScore = &hostScore;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            gotoxy(2, board ? board->rows() + 15 : 15);
            std::cout << "¡Has ganado! Ingrese su nombre (Enter para '" << winnerLabel << "'): ";
            std::cout.flush();
            std::getline(std::cin, winnerName);
            if (winnerName.empty()) winnerName = winnerLabel;
            winnerScore->won = true;
            ScoreCalculator::saveMultiplayerScoreToCSV(*winnerScore, winnerName);
            gotoxy(2, board ? board->rows() + 17 : 17);
            std::cout << "Puntaje guardado." << std::endl;
            break;
        }  
    } 
  
        // Cambiar turno  
        turnHost = !turnHost;  
    }  
    
  
    // Esperar input antes de limpiar recursos
    gotoxy(2, board ? board->rows() + 10 : 2);
    std::cout << "Presione Enter para volver al menú principal..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    // Limpieza de recursos
    close(sockfd);
    delete board;
    board = nullptr;
}
