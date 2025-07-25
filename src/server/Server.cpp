#include "server/Server.h"
#include "common/ConsoleUtils.h"
#include "common/NetworkCommon.h"
#include "common/ScoreSystem.h"
#include <arpa/inet.h>
#include <ctime>
#include <cstring>
#include <iostream>
#include <limits>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <chrono>



Server::Server(const std::string& name, int rows, int cols, int mines, int difficulty)
  : roomName(name),
    R(rows), C(cols), M(mines), difficulty(difficulty),
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
        std::cerr << "/nError creando socket UDP" << std::endl;  
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
        std::cerr << "/nError en bind UDP" << std::endl;  
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

void Server::gameLoop() {  
    int srvSock = socket(AF_INET, SOCK_STREAM, 0);  
    if (srvSock == -1) {  
        std::cerr << "\nError creando socket TCP" << std::endl;  
        return;  
    }  

    //reutilizar el puerto  
    int opt = 1;  
    if (setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {  
        std::cerr << "\nError configurando SO_REUSEADDR" << std::endl;  
        close(srvSock);  
        return;  
    }
      
    sockaddr_in srvAddr{};  
    srvAddr.sin_family = AF_INET;  
    srvAddr.sin_port = htons(GAME_PORT_BASE);  
    srvAddr.sin_addr.s_addr = INADDR_ANY;  
      
    if (bind(srvSock, (sockaddr*)&srvAddr, sizeof(srvAddr)) == -1) {  
        std::cerr << "\nError en bind TCP" << std::endl;  
        close(srvSock);  
        return;  
    }  
      
    if (listen(srvSock, 1) == -1) {  
        std::cerr << "\nError en listen" << std::endl;  
        close(srvSock);  
        return;  
    }  
  
    clearScreen();   
    gotoxy(1, 1);  
    drawFrameAroundBoard(4, 2, board.cols(), board.rows()); 
    board.drawGotoxy(4, 2);  
    gotoxy(2 , board.rows() + 3);  
    std::cout << "Esperando cliente…\n";  
    int clientSock = accept(srvSock, nullptr, nullptr);  
    if (clientSock == -1) {  
        std::cerr << "Error aceptando conexión" << std::endl;  
        close(srvSock);  
        return;  
    }  
    gotoxy(2, board.rows() + 4);
    std::cout << "Cliente conectado\n";  
  
    // Enviar configuración del juego  
    GameInit gi;  
    gi.seed = htonl(seed);  
    gi.rows = static_cast<uint8_t>(R);  
    gi.cols = static_cast<uint8_t>(C);  
    gi.mines = static_cast<uint8_t>(M);  
    gi.difficulty = static_cast<uint8_t>(difficulty); // Propagar dificultad real
    if (NetworkUtils::safeSend(clientSock, &gi, sizeof(gi)) <= 0) {  
        std::cerr << "\nError enviando configuración del juego" << std::endl;  
        close(clientSock);  
        close(srvSock);  
        return;  
    }  
  
    bool turnHost = true; 
    // Variables de puntuación para ambos jugadores  
    auto startTime = std::chrono::steady_clock::now();  
    int hostClicks = 0, clientClicks = 0;  
    int hostFlags = 0, clientFlags = 0;  
    // Usar el miembro difficulty del Server, no una variable local

    auto lastTimeUpdate = std::chrono::steady_clock::now();  
    bool statsDisplayed = false;

    while (true) {  
        // Actualizar tiempo al inicio del bucle  
        auto currentTime = std::chrono::steady_clock::now();  
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();  
        
        // Actualizar tiempo cada segundo  
        if (std::chrono::duration<double>(currentTime - lastTimeUpdate).count() >= 1.0) {  
            if (statsDisplayed) {  
                ScoreCalculator::updateTimeOnly(elapsedTime, board.rows());  
            }  
            lastTimeUpdate = currentTime;  
        } 
        Move mv{};  
          
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
                    // Mostrar estadísticas en vivo  
                    // REEMPLAZAR cualquier llamada existente a displayLiveStats con:  
                    ScoreCalculator::displayLiveStats(hostClicks, hostFlags, elapsedTime,  
                                                    clientClicks, clientFlags,   
                                                    "HOST", "CLIENT", board.rows());  
                    statsDisplayed = true;
                    if (board.rows() >= 14) gotoxy(4 + cursorCol * 3, 1 + cursorRow);    
                    else gotoxy(4 + cursorCol * 3, 2 + cursorRow); 
                    std::cout << "\033[35m◉\033[0m";
                    // highlightCell(cursorRow, cursorCol, "[◉]");  
  
                    gotoxy(2, board.rows() + 9);  // Era board.rows() + 6  
                    std::cout << "\033[92m┌─ CONTROLES ──────────────────────────────┐\033[0m\n";  
                    gotoxy(2, board.rows() + 10);  // Era board.rows() + 7  
                    std::cout << "\033[92m│ \033[97m⬆️⬇️⬅️➡️ Mover  \033[93mR\033[97m Revelar  \033[93mF\033[97m Bandera \033[93mQ\033[97m Salir \033[92m│\033[0m\n";  
                    gotoxy(2, board.rows() + 11);  // Era board.rows() + 8  
                    std::cout << "\033[92m└──────────────────────────────────────────┘\033[0m";  
                }  
  
                if (key == KEY_FLAG) {  
                    mv.row = static_cast<uint8_t>(cursorRow);  
                    mv.col = static_cast<uint8_t>(cursorCol);  
                    mv.isFlag = 1;  
                    hostClicks++;  
                    hostFlags++;  
                    break;  
                }  
                else if (key == KEY_ENTER) {  
                    mv.row = static_cast<uint8_t>(cursorRow);  
                    mv.col = static_cast<uint8_t>(cursorCol);  
                    mv.isFlag = 0;  
                    hostClicks++;  
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
              
            // drawStatusBar("⏳ Esperando movimiento del rival…", board.rows() + 5);  
              
            // Enviar movimiento al cliente con verificación de errores  
            if (NetworkUtils::safeSend(clientSock, &mv, sizeof(mv)) <= 0) {  
                std::cerr << "Error enviando movimiento al cliente" << std::endl;  
                break;  
            }  
              
        } else {  
            // Turno del cliente - recibir movimiento  
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
              
            // Validar movimiento recibido  
            if (!validateMove(mv)) {  
                std::cerr << "Movimiento inválido recibido del cliente: ("   
                         << (int)mv.row << "," << (int)mv.col << "), flag=" << (int)mv.isFlag << std::endl;  
                continue; // Continuar esperando un movimiento válido  
            }  

            if (mv.isFlag) {  
                clientClicks++;  
                clientFlags++;  
            } else {  
                clientClicks++;  
}

            // Después de recibir un movimiento del rival  
            // highlightCell(mv.row, mv.col, mv.isFlag ? "[🚩]" : "[!]");  
            // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        }  
  
        // Aplicar movimiento al tablero  
        if (mv.isFlag) {  
            board.toggleFlag(mv.row, mv.col);  
        } else {  
            board.reveal(mv.row, mv.col);  
        }  
        
        // Actualizar display del tablero  
        clearScreen();  
        gotoxy(1, 1);  
        drawFrameAroundBoard(4, 2, board.cols(), board.rows());  
        board.drawGotoxy(4, 2);  
        
        // SOLO verificar condiciones de fin de juego si fue una revelación  
        if (!mv.isFlag) {  
        if (board.isMine(mv.row, mv.col)) {  
            std::string result = turnHost ? "Has perdido💣" : "Has ganado🏁";  
            showAllMines(board, result);  
            
            // Calcular puntuaciones para ambos jugadores  
            auto endTime = std::chrono::steady_clock::now();  
            double gameTime = std::chrono::duration<double>(endTime - startTime).count();  
            
            // Puntuación del host  
            bool hostWon = !turnHost;  
            GameScore hostScore = ScoreCalculator::calculateScore(difficulty, R, C,   
                                                                gameTime, hostClicks, hostFlags, hostWon);  
            
            // Puntuación del cliente    
            bool clientWon = turnHost;  
            GameScore clientScore = ScoreCalculator::calculateScore(difficulty, R, C,  
                                                                gameTime, clientClicks, clientFlags, clientWon);  
            
            MultiplayerScore mscore;  
            mscore.hostScore = hostScore.totalScore;  
            mscore.clientScore = clientScore.totalScore;  
            mscore.hostWon = hostWon;  
            mscore.clientWon = clientWon;  
            mscore.gameTime = gameTime;  
            mscore.hostClicks = hostClicks;  
            mscore.clientClicks = clientClicks;  
            mscore.hostFlags = hostFlags;  
            mscore.clientFlags = clientFlags;  
            
            // Enviar la estructura al cliente  
            send(clientSock, &mscore, sizeof(mscore), 0);
            // Mostrar resultados  
            ScoreCalculator::displayMultiplayerResults(hostScore, clientScore, "HOST", "CLIENT");  
            
            // Agregar estas líneas:  
            if (hostWon) {  
                gotoxy(2, board.rows() + 15);  
                std::cout << "¡HOST ha ganado! Ingrese su nombre (Enter para 'HOST'): ";  
                std::string winnerName;  
                if (std::cin.peek() == '\n') std::cin.ignore();
                std::getline(std::cin, winnerName);  
                if (winnerName.empty()) winnerName = "HOST";  
                hostScore.won = true;
                hostScore.playerName = winnerName;
                // Mapear dificultad numérica a string legible
                switch (difficulty) {
                    case 1: hostScore.difficulty = "facil"; break;
                    case 2: hostScore.difficulty = "medio"; break;
                    case 3: hostScore.difficulty = "dificil"; break;
                    default: hostScore.difficulty = "desconocido"; break;
                }
                ScoreCalculator::saveMultiplayerScoreToCSV(hostScore, winnerName);  
            } else if (clientWon) {  
                gotoxy(2, board.rows() + 15);  
                std::cout << "¡CLIENT ha ganado! El ganador podrá ingresar su nombre desde su cliente.";  
            }
            break;  
        }  
        
        if (board.allSafeRevealed()) {  
            std::string result = turnHost ? "Has ganado🏁" : "Has perdido💣";  
            showAllMines(board, result);  
            
            // Calcular puntuaciones para victoria completa  
            auto endTime = std::chrono::steady_clock::now();  
            double gameTime = std::chrono::duration<double>(endTime - startTime).count();  
            
            bool hostWon = turnHost;  
            GameScore hostScore = ScoreCalculator::calculateScore(difficulty, R, C,  
                                                                gameTime, hostClicks, hostFlags, hostWon);  
            
            bool clientWon = !turnHost;  
            GameScore clientScore = ScoreCalculator::calculateScore(difficulty, R, C,  
                                                                gameTime, clientClicks, clientFlags, clientWon);  
            
            ScoreCalculator::displayMultiplayerResults(hostScore, clientScore, "HOST", "CLIENT");  
            
            break;  
        }  
    }  
          
        // Cambiar turno  
        turnHost = !turnHost;  
    }  

  
    // Esperar input antes de limpiar recursos
    gotoxy(2, board.rows() + 10);
    std::cout << "Presione Enter para volver al menú principal..." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    // Limpieza de recursos
    close(clientSock);
    close(srvSock);
}

void Server::run() {  
    discoveryThread = std::thread(&Server::discoveryListener, this);  
    gameLoop();  
    stop();  
}